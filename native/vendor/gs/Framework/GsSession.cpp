#include "commons/GsSession.h"
#include <algorithm>

namespace cc::Gs {

void GsModules::shutdown() {
    // SDK callbacks are unregistered before their owners and the SDK are released.
    if (utils) utils->shutdown();
    if (remoteStorage) remoteStorage->shutdown();
    if (friends) friends->shutdown();
    if (achievements) achievements->shutdown();
    if (stats) stats->shutdown();
    if (account) account->shutdown();
    utils.reset();
    remoteStorage.reset();
    friends.reset();
    achievements.reset();
    stats.reset();
    account.reset();
}

GsSession::GsSession(std::unique_ptr<GsPlatform> platform) : _platform(std::move(platform)) {}
GsSession::~GsSession() { close(); }

void GsSession::init(OnComplete callback) {
    if (isClosed() || isClosing()) { callback.failure({GsErrorCode::Cancelled, "Services closed"}); return; }
    if (isActive()) {
        Dispatch dispatch(*this);
        callback.success();
        return;
    }
    if (_state == State::Initializing) {
        _initWaiters.push_back(std::move(callback));
        return;
    }
    if (_dispatchDepth != 0) { callback.failure({GsErrorCode::Busy, "Services operation already in progress"}); return; }
    Dispatch dispatch(*this);
    _state = State::Initializing;
    _initWaiters.push_back(std::move(callback));
    _initialization = std::make_shared<Initialization>();
    std::weak_ptr<Initialization> token = _initialization;
    _platformStarted = true;
    _platform->initialize(_modules, OnComplete{
        [token]() {
            if (auto result = token.lock()) result->completed = true;
        },
        [token](const GsError& error) {
            if (auto result = token.lock()) {
                result->error = error;
                result->completed = true;
            }
        }});
    // Never clean up platform resources from inside its completion callback.
    finishInitialization();
}

void GsSession::finishInitialization() {
    if (_state != State::Initializing || !_initialization || !_initialization->completed) return;
    const auto error = _initialization->error;
    _initialization.reset();
    if (error) {
        _modules.shutdown();
        _platform->shutdown();
        _platformStarted = false;
        // shutdown may have requested close through an engine callback.
        if (_state == State::Initializing) _state = State::Created;
    } else {
        _state = State::Ready;
        _gate->active = true;
    }
    auto waiters = std::move(_initWaiters);
    _initWaiters.clear();
    for (auto& callback : waiters) {
        if (isClosing() || isClosed()) callback.failure({GsErrorCode::Cancelled, "Services closed"});
        else if (error) callback.failure(*error);
        else callback.success();
    }
}

void GsSession::close() {
    if (_state == State::Closed || _state == State::Closing) return;
    _state = State::Closing;
    _gate->active = false;
    if (_dispatchDepth == 0) finishClose();
}

void GsSession::finishClose() {
    if (_finishingClose) return;
    _finishingClose = true;
    // Move the pending list before invoking cancellation handlers. Reentrant
    // close/getServices calls cannot mutate this list or reopen this session.
    // Invalidate completion before cancelling waiters or releasing the platform.
    _initialization.reset();
    auto initWaiters = std::move(_initWaiters);
    _initWaiters.clear();
    for (auto& callback : initWaiters) callback.failure({GsErrorCode::Cancelled, "Services closed"});
    auto pending = std::move(_pending);
    for (auto& entry : pending) {
        if (auto request = entry.callback.lock()) request->cancel({GsErrorCode::Cancelled, "Services closed"});
    }
    _modules.shutdown();
    if (_platformStarted) {
        _platform->shutdown();
        _platformStarted = false;
    }
    _platform.reset();
    _state = State::Closed;
    _finishingClose = false;
}

GsSession::Dispatch::~Dispatch() {
    if (--_session->_dispatchDepth == 0 && _session->isClosing()) _session->finishClose();
}

void GsSession::tick(float dt) {
    if (!isActive() && _state != State::Initializing) return;
    Dispatch dispatch(*this);
    finishInitialization();
    if (!isActive() && _state != State::Initializing) return;
    _platform->pump(dt);
    finishInitialization();
    if (isActive()) expireRequests();
}

void GsSession::restartAppIfNecessary(const AppId& appId, OnRestartRequired callback) {
    if (isClosed() || isClosing()) { callback.failure({GsErrorCode::Cancelled, "Services closed"}); return; }
    if (_state != State::Created) {
        callback.failure({GsErrorCode::InvalidArgument, "Launcher restart must be checked before init"}); return;
    }
    if (_dispatchDepth != 0) { callback.failure({GsErrorCode::Busy, "Services operation already in progress"}); return; }
    Dispatch dispatch(*this);
    _platform->restartAppIfNecessary(appId, std::move(callback));
}

void GsSession::track(const std::shared_ptr<PendingCallback>& pending,
                      std::optional<std::chrono::milliseconds> timeout) {
    if (!pending || pending->completed) return;
    pending->gate = _gate;
    _pending.erase(std::remove_if(_pending.begin(), _pending.end(), [](const auto& entry) {
        auto request = entry.callback.lock();
        return !request || request->completed;
    }), _pending.end());
    _pending.push_back({pending, timeout ? std::make_optional(std::chrono::steady_clock::now() + *timeout) : std::nullopt});
}

void GsSession::expireRequests() {
    if (_pending.empty()) return;
    const auto now = std::chrono::steady_clock::now();
    if (now < _nextPendingCheck) return;
    _nextPendingCheck = now + std::chrono::milliseconds(500);
    std::vector<std::shared_ptr<PendingCallback>> expired;
    for (auto it = _pending.begin(); it != _pending.end();) {
        auto request = it->callback.lock();
        if (!request || request->completed) {
            it = _pending.erase(it);
        } else if (it->deadline && now >= *it->deadline) {
            expired.push_back(std::move(request));
            it = _pending.erase(it);
        } else {
            ++it;
        }
    }
    // A callback may enqueue requests or close the session. Do not retain iterators
    // across notifications; closing also cancels the rest of this detached batch.
    for (auto& request : expired) {
        if (isClosing() || isClosed()) request->cancel({GsErrorCode::Cancelled, "Services closed"});
        else request->cancel({GsErrorCode::Timeout, "Request timed out; platform work may still be running"});
    }
}

} // namespace cc::Gs

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
    utils.reset();
    remoteStorage.reset();
    friends.reset();
    achievements.reset();
    stats.reset();
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
    if (_dispatchDepth != 0) { callback.failure({GsErrorCode::Busy, "Services initialization is already in progress"}); return; }
    Dispatch dispatch(*this);
    const auto error = _platform->initialize(_modules);
    if (error) {
        _modules.shutdown();
        callback.failure(isClosed() || isClosing() ? GsError{GsErrorCode::Cancelled, "Services closed"} : *error);
        return;
    }
    _sdkInitialized = true;
    if (_state != State::Created) { callback.failure({GsErrorCode::Cancelled, "Services closed during initialization"}); return; }
    _state = State::Ready;
    _gate->active = true;
    callback.success();
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
    auto pending = std::move(_pending);
    for (auto& weak : pending) {
        if (auto request = weak.lock()) request->cancel({GsErrorCode::Cancelled, "Services closed"});
    }
    _modules.shutdown();
    if (_sdkInitialized) {
        _platform->shutdown();
        _sdkInitialized = false;
    }
    _platform.reset();
    _state = State::Closed;
    _finishingClose = false;
}

GsSession::Dispatch::~Dispatch() {
    if (--_session->_dispatchDepth == 0 && _session->isClosing()) _session->finishClose();
}

void GsSession::tick(float dt) {
    if (!isActive()) return;
    Dispatch dispatch(*this);
    _platform->pump(dt);
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

void GsSession::track(const std::shared_ptr<PendingCallback>& pending) {
    if (!pending) return;
    pending->gate = _gate;
    _pending.erase(std::remove_if(_pending.begin(), _pending.end(), [](const auto& weak) {
        auto request = weak.lock();
        return !request || request->completed;
    }), _pending.end());
    _pending.emplace_back(pending);
}

} // namespace cc::Gs

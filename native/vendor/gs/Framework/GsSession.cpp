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

bool GsSession::init() {
    if (isActive()) return true;
    if (_state != State::Created || _dispatchDepth != 0) return false;
    Dispatch dispatch(*this);
    if (!_platform->initialize(_modules)) {
        _modules.shutdown();
        return false;
    }
    _sdkInitialized = true;
    if (_state != State::Created) return false;
    _state = State::Active;
    _gate->active = true;
    return true;
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
        if (auto request = weak.lock()) request->cancel("Services closed");
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

bool GsSession::restartAppIfNecessary(const AppId& appId) {
    if (_state != State::Created) return false;
    Dispatch dispatch(*this);
    return _platform->restartAppIfNecessary(appId);
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

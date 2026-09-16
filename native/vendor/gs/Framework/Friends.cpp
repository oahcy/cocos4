#include "Friends.h"
#include "commons/GsSession.h"

namespace cc::Gs {
IFriends::IFriends(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IFriends::~IFriends() = default;

void IFriends::getLocalUser(OnUserProfile callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Friends unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->getLocalUser(std::move(callback));
}

void IFriends::getFriends(OnFriends callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Friends unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->getFriends(std::move(callback));
}

void IFriends::getAvatar(const AccountId& userId, AvatarSize size, OnAvatarLoaded callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Friends unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->getAvatar(userId, size, std::move(callback));
}

void IFriends::getGroups(OnFriendGroups callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Friends unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->getGroups(std::move(callback));
}

void IFriends::setRichPresence(const std::string& key, const std::string& value, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Friends unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->setRichPresence(key, value, std::move(callback));
}

void IFriends::clearRichPresence(OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Friends unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->clearRichPresence(std::move(callback));
}

void IFriends::getRichPresence(const AccountId& userId, const std::string& key, OnPresenceValue callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Friends unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->getRichPresence(userId, key, std::move(callback));
}

void IFriends::openOverlay(OverlayDialog dialog, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Friends unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->openOverlay(dialog, std::move(callback));
}

void IFriends::openWebPage(const std::string& url, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Friends unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->openWebPage(url, std::move(callback));
}

void IFriends::setOnJoinRequested(OnJoinRequested delegate) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) return;
    delegate.setGate(_session->gate());
    backend->setOnJoinRequested(std::move(delegate));
}
} // namespace cc::Gs

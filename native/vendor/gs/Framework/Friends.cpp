#include "Friends.h"
#include "commons/GsSession.h"

namespace cc::Gs {

IFriends::IFriends(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IFriends::~IFriends() = default;

std::string IFriends::getPersonaName() {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        return {};
    }
    return backend->getPersonaName();
}

FriendListResult IFriends::getFriends(FriendFlags friendFlags) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        return {};
    }
    return backend->getFriends(friendFlags);
}

void IFriends::requestAvatar(const AccountId& userId, AvatarSize size, OnAvatarLoaded callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        callback.failure("Friends unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->requestAvatar(userId, size, std::move(callback));
}

FriendsGroupListResult IFriends::getFriendsGroups() {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        return {};
    }
    return backend->getFriendsGroups();
}

bool IFriends::setRichPresence(const std::string& key, const std::string& value) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        return {};
    }
    return backend->setRichPresence(key, value);
}

void IFriends::clearRichPresence() {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        return;
    }
    backend->clearRichPresence();
}

std::string IFriends::getFriendRichPresence(const AccountId& userId, const std::string& key) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        return {};
    }
    return backend->getFriendRichPresence(userId, key);
}

void IFriends::activateGameOverlay(OverlayDialog dialog) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        return;
    }
    backend->activateGameOverlay(dialog);
}

void IFriends::activateGameOverlayToWebPage(const std::string& url) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        return;
    }
    backend->activateGameOverlayToWebPage(url);
}

void IFriends::setOnGameRichPresenceJoinRequested(OnGameRichPresenceJoinRequested delegate) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->friends();
    if (!backend) {
        return;
    }
    delegate.setGate(_session->gate());
    backend->setOnGameRichPresenceJoinRequested(std::move(delegate));
}

} // namespace cc::Gs

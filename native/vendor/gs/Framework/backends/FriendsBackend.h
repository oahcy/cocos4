#pragma once

#include "../Friends.h"
#include "GsBackend.h"

namespace cc::Gs {

class IFriendsBackend : public GsBackend {
public:
    virtual std::string getPersonaName() = 0;
    virtual FriendListResult getFriends(FriendFlags friendFlags) = 0;
    virtual void requestAvatar(const AccountId& userId, AvatarSize size, OnAvatarLoaded callback) = 0;
    virtual FriendsGroupListResult getFriendsGroups() = 0;
    virtual bool setRichPresence(const std::string& key, const std::string& value) = 0;
    virtual void clearRichPresence() = 0;
    virtual std::string getFriendRichPresence(const AccountId& userId, const std::string& key) = 0;
    virtual void activateGameOverlay(OverlayDialog dialog) = 0;
    virtual void activateGameOverlayToWebPage(const std::string& url) = 0;
    virtual void setOnGameRichPresenceJoinRequested(OnGameRichPresenceJoinRequested delegate) = 0;
};

} // namespace cc::Gs

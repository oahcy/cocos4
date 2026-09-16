#pragma once
#include "../Friends.h"
#include "GsBackend.h"

namespace cc::Gs {
class IFriendsBackend : public GsBackend {
public:
    virtual void getLocalUser(OnUserProfile callback) = 0;
    virtual void getFriends(OnFriends callback) = 0;
    virtual void getAvatar(const AccountId& userId, AvatarSize size, OnAvatarLoaded callback) = 0;
    virtual void getGroups(OnFriendGroups callback) { callback.failure({GsErrorCode::NotSupported, "getGroups is not supported"}); }
    virtual void setRichPresence(const std::string& key, const std::string& value, OnComplete callback) { callback.failure({GsErrorCode::NotSupported, "setRichPresence is not supported"}); }
    virtual void clearRichPresence(OnComplete callback) { callback.failure({GsErrorCode::NotSupported, "clearRichPresence is not supported"}); }
    virtual void getRichPresence(const AccountId& userId, const std::string& key, OnPresenceValue callback) { callback.failure({GsErrorCode::NotSupported, "getRichPresence is not supported"}); }
    virtual void openOverlay(OverlayDialog dialog, OnComplete callback) { callback.failure({GsErrorCode::NotSupported, "openOverlay is not supported"}); }
    virtual void openWebPage(const std::string& url, OnComplete callback) { callback.failure({GsErrorCode::NotSupported, "openWebPage is not supported"}); }
    virtual void setOnJoinRequested(OnJoinRequested delegate) = 0;
};
} // namespace cc::Gs

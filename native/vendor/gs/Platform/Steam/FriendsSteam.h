#pragma once

#include <steam_api.h>
#include <chrono>
#include "../../Framework/backends/FriendsBackend.h"

namespace cc::Gs {

class FriendsSteam : public IFriendsBackend {
public:
    FriendsSteam()
        : _cbAvatarLoaded(this, &FriendsSteam::onAvatarImageLoaded)
        , _cbGameJoinRequested(this, &FriendsSteam::onGameRichPresenceJoinRequested) {}

    void shutdown() override;
    void expireAvatarRequests(std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());

    void getLocalUser(OnUserProfile callback) override;
    void getFriends(OnFriends callback) override;
    void getAvatar(const AccountId& userId, AvatarSize size, OnAvatarLoaded callback) override;
    void getGroups(OnFriendGroups callback) override;
    void setRichPresence(const std::string& key, const std::string& value, OnComplete callback) override;
    void clearRichPresence(OnComplete callback) override;
    void getRichPresence(const AccountId& userId, const std::string& key, OnPresenceValue callback) override;
    void openOverlay(OverlayDialog dialog, OnComplete callback) override;
    void openWebPage(const std::string& url, OnComplete callback) override;
    void setOnJoinRequested(OnJoinRequested delegate) override;

private:
    AvatarImage fetchAvatar(int handle);

    STEAM_CALLBACK(FriendsSteam, onAvatarImageLoaded, AvatarImageLoaded_t, _cbAvatarLoaded);
    STEAM_CALLBACK(FriendsSteam, onGameRichPresenceJoinRequested, GameRichPresenceJoinRequested_t, _cbGameJoinRequested);

    struct PendingAvatar {
        CSteamID steamId;
        AvatarSize size;
        OnAvatarLoaded callback;
        std::chrono::steady_clock::time_point deadline;
    };
    std::vector<PendingAvatar> _pendingAvatars;
    OnJoinRequested _gameRichPresenceJoinDelegate;
};

} // namespace cc::Gs

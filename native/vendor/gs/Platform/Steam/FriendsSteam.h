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

    std::string getPersonaName() override;

    FriendListResult getFriends(FriendFlags friendFlags) override;

    void requestAvatar(const AccountId& userId, AvatarSize size, OnAvatarLoaded callback) override;

    FriendsGroupListResult getFriendsGroups() override;

    bool setRichPresence(const std::string& key, const std::string& value) override;
    void clearRichPresence() override;
    std::string getFriendRichPresence(const AccountId& userId, const std::string& key) override;

    void activateGameOverlay(OverlayDialog dialog) override;
    void activateGameOverlayToWebPage(const std::string& url) override;

    void setOnGameRichPresenceJoinRequested(OnGameRichPresenceJoinRequested delegate) override;

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
    OnGameRichPresenceJoinRequested _gameRichPresenceJoinDelegate;
};

} // namespace cc::Gs

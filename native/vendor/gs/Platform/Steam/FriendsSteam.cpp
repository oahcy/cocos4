#include "FriendsSteam.h"
#include "../../Framework/JsbConversions.h"

#include "base/Log.h"
#include <string>

namespace cc::Gs {

static bool parseSteamId(const std::string& str, CSteamID& outId) {
    try {
        outId = CSteamID(std::stoull(str));
        return outId.IsValid();
    } catch (...) {
        return false;
    }
}

static int toSteamFriendFlags(FriendFlags flags) {
    if (flags == FriendFlags::All) return k_EFriendFlagAll;
    int out = k_EFriendFlagNone;
    if (hasFlag(flags, FriendFlags::Immediate)) out |= k_EFriendFlagImmediate;
    if (hasFlag(flags, FriendFlags::Blocked)) out |= k_EFriendFlagBlocked;
    if (hasFlag(flags, FriendFlags::FriendshipRequested)) out |= k_EFriendFlagFriendshipRequested;
    if (hasFlag(flags, FriendFlags::RequestingFriendship)) out |= k_EFriendFlagRequestingFriendship;
    if (hasFlag(flags, FriendFlags::ClanMember)) out |= k_EFriendFlagClanMember;
    if (hasFlag(flags, FriendFlags::OnGameServer)) out |= k_EFriendFlagOnGameServer;
    return out;
}

// Returns nullptr for dialogs Steam has no equivalent for.
static const char* toSteamOverlayDialog(OverlayDialog dialog) {
    switch (dialog) {
    case OverlayDialog::Friends:           return "friends";
    case OverlayDialog::Community:         return "community";
    case OverlayDialog::Players:           return "players";
    case OverlayDialog::Settings:          return "settings";
    case OverlayDialog::OfficialGameGroup: return "officialgamegroup";
    case OverlayDialog::Stats:             return "stats";
    case OverlayDialog::Achievements:      return "achievements";
    }
    return nullptr;
}

void FriendsSteam::shutdown() {
    // Mark shut down FIRST so any re-entrant call from dropped JS callbacks
    // hits isShutdown() and fails fast instead of touching a torn-down session.
    Super::shutdown();
    // Unregister Steam callbacks before SteamAPI_Shutdown so their destructors
    // (which may run later, after the session is gone) are safe no-ops.
    _cbAvatarLoaded.Unregister();
    _cbGameJoinRequested.Unregister();
    // Drop still-pending avatar requests without invoking JS: running user
    // callbacks inside the teardown stack would be re-entrant, and the TS
    // lifecycle token has already invalidated every helper, so the JS side can
    // only log, never act.
    if (!_pendingAvatars.empty()) {
        CC_LOG_WARNING("[Friends] Cancelling %zu pending avatar request(s) during shutdown", _pendingAvatars.size());
        for (auto& pending : _pendingAvatars) {
            pending.callback.reset();
        }
        _pendingAvatars.clear();
    }
    // Release the rooted JS listener for join requests.
    _gameRichPresenceJoinDelegate.reset();
}

std::string FriendsSteam::getPersonaName() {
    if (isShutdown()) return "";
    return SteamFriends()->GetPersonaName();
}

FriendListResult FriendsSteam::getFriends(FriendFlags friendFlags) {
    if (isShutdown()) return {};
    const int steamFlags = toSteamFriendFlags(friendFlags);
    FriendListResult result;
    int count = SteamFriends()->GetFriendCount(steamFlags);
    for (int i = 0; i < count; i++) {
        CSteamID id = SteamFriends()->GetFriendByIndex(i, steamFlags);
        if (!id.IsValid()) continue;

        FriendInfo info;
        info.userId = std::to_string(id.ConvertToUint64());
        info.personaName = SteamFriends()->GetFriendPersonaName(id);
        const char* nick = SteamFriends()->GetPlayerNickname(id);
        info.nickname = nick ? nick : "";
        info.personaState = static_cast<PersonaState>(SteamFriends()->GetFriendPersonaState(id));
        result.Friends.push_back(std::move(info));
    }
    return result;
}

AvatarImage FriendsSteam::fetchAvatar(int handle) {
    AvatarImage img;
    if (handle <= 0) return img;

    uint32 w = 0, h = 0;
    if (!SteamUtils()->GetImageSize(handle, &w, &h) || w == 0 || h == 0) return img;

    img.width = static_cast<int>(w);
    img.height = static_cast<int>(h);
    img.data.resize(w * h * 4);
    if (!SteamUtils()->GetImageRGBA(handle, img.data.data(), static_cast<int>(img.data.size()))) {
        img.data.clear();
        img.width = 0;
        img.height = 0;
    }
    return img;
}

void FriendsSteam::requestAvatar(const AccountId& userId, AvatarSize size, OnAvatarLoaded callback) {
    if (isShutdown()) {
        callback.failure("Services shut down");
        return;
    }
    CSteamID id;
    if (!parseSteamId(userId, id)) {
        callback.failure("Invalid userId");
        return;
    }

    int handle = 0;
    switch (size) {
    case AvatarSize::Small:  handle = SteamFriends()->GetSmallFriendAvatar(id); break;
    case AvatarSize::Medium: handle = SteamFriends()->GetMediumFriendAvatar(id); break;
    case AvatarSize::Large:  handle = SteamFriends()->GetLargeFriendAvatar(id); break;
    }

    if (handle > 0) {
        AvatarImage img = fetchAvatar(handle);
        callback.success(img);
    } else if (size == AvatarSize::Large && (handle == 0 || handle == -1)) {
        // Prevent memory leak: cap the pending queue to avoid infinite accumulation
        // if the Steam network fails to trigger AvatarImageLoaded_t.
        if (_pendingAvatars.size() >= 50) {
            CC_LOG_WARNING("[Friends] Avatar request queue is full. Dropping the oldest request.");
            _pendingAvatars.front().callback.failure("Avatar request timeout or queue full");
            _pendingAvatars.erase(_pendingAvatars.begin());
        }
        _pendingAvatars.push_back({id, size, std::move(callback)});
    } else {
        callback.failure("No avatar available");
    }
}

void FriendsSteam::onAvatarImageLoaded(AvatarImageLoaded_t* pParam) {
    for (auto it = _pendingAvatars.begin(); it != _pendingAvatars.end(); ) {
        if (it->steamId == pParam->m_steamID) {
            int handle = 0;
            switch (it->size) {
            case AvatarSize::Small:  handle = SteamFriends()->GetSmallFriendAvatar(it->steamId); break;
            case AvatarSize::Medium: handle = SteamFriends()->GetMediumFriendAvatar(it->steamId); break;
            case AvatarSize::Large:  handle = SteamFriends()->GetLargeFriendAvatar(it->steamId); break;
            }
            if (handle > 0) {
                AvatarImage img = fetchAvatar(handle);
                it->callback.success(img);
            } else {
                it->callback.failure("Avatar load failed");
            }
            it = _pendingAvatars.erase(it);
        } else {
            ++it;
        }
    }
}

FriendsGroupListResult FriendsSteam::getFriendsGroups() {
    if (isShutdown()) return {};
    FriendsGroupListResult result;
    int groupCount = SteamFriends()->GetFriendsGroupCount();
    for (int i = 0; i < groupCount; i++) {
        FriendsGroupID_t gid = SteamFriends()->GetFriendsGroupIDByIndex(i);
        if (gid == k_FriendsGroupID_Invalid) continue;

        FriendsGroupInfo group;
        group.groupId = gid;
        const char* name = SteamFriends()->GetFriendsGroupName(gid);
        group.groupName = name ? name : "";

        int memberCount = SteamFriends()->GetFriendsGroupMembersCount(gid);
        if (memberCount > 0) {
            std::vector<CSteamID> memberIds(memberCount);
            SteamFriends()->GetFriendsGroupMembersList(gid, memberIds.data(), memberCount);
            for (int m = 0; m < memberCount; m++) {
                group.members.push_back(std::to_string(memberIds[m].ConvertToUint64()));
            }
        }
        result.Groups.push_back(std::move(group));
    }
    return result;
}

bool FriendsSteam::setRichPresence(const std::string& key, const std::string& value) {
    if (isShutdown()) return false;
    return SteamFriends()->SetRichPresence(key.c_str(), value.c_str());
}

void FriendsSteam::clearRichPresence() {
    if (isShutdown()) return;
    SteamFriends()->ClearRichPresence();
}

std::string FriendsSteam::getFriendRichPresence(const AccountId& userId, const std::string& key) {
    if (isShutdown()) return "";
    CSteamID id;
    if (!parseSteamId(userId, id)) return "";
    const char* val = SteamFriends()->GetFriendRichPresence(id, key.c_str());
    return val ? val : "";
}

void FriendsSteam::activateGameOverlay(OverlayDialog dialog) {
    if (isShutdown()) return;
    const char* name = toSteamOverlayDialog(dialog);
    if (!name) {
        CC_LOG_ERROR("[Friends] activateGameOverlay: dialog %d is not supported on Steam", static_cast<int>(dialog));
        return;
    }
    SteamFriends()->ActivateGameOverlay(name);
}

void FriendsSteam::activateGameOverlayToWebPage(const std::string& url) {
    if (isShutdown()) return;
    SteamFriends()->ActivateGameOverlayToWebPage(url.c_str());
}

void FriendsSteam::setOnGameRichPresenceJoinRequested(OnGameRichPresenceJoinRequested delegate) {
    if (isShutdown()) return;
    _gameRichPresenceJoinDelegate = std::move(delegate);
}

void FriendsSteam::onGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t* pParam) {
    if (_gameRichPresenceJoinDelegate) {
        std::string friendId = std::to_string(pParam->m_steamIDFriend.ConvertToUint64());
        std::string connectStr = pParam->m_rgchConnect;
        _gameRichPresenceJoinDelegate.invoke(friendId, connectStr);
    }
}

} // namespace cc::Gs

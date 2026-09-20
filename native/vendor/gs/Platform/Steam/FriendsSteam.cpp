#include "FriendsSteam.h"


#include "base/Log.h"
#include <string>
#include <algorithm>
#include <limits>

namespace cc::Gs {

static bool parseSteamId(const std::string& str, CSteamID& outId) {
    try {
        if (str.empty() || str.find_first_not_of("0123456789") != std::string::npos) return false;
        outId = CSteamID(std::stoull(str));
        return outId.IsValid();
    } catch (...) {
        return false;
    }
}

static PresenceState toPresence(EPersonaState state) {
    switch (state) {
    case k_EPersonaStateOffline: return PresenceState::Offline;
    case k_EPersonaStateOnline:
    case k_EPersonaStateLookingToTrade:
    case k_EPersonaStateLookingToPlay: return PresenceState::Online;
    case k_EPersonaStateAway:
    case k_EPersonaStateSnooze: return PresenceState::Away;
    case k_EPersonaStateBusy: return PresenceState::Busy;
    default: return PresenceState::Unknown;
    }
}

static bool validText(const std::string& value) {
    return !value.empty() && value.find('\0') == std::string::npos;
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
    // Unregister Steam callbacks before SteamAPI_Shutdown so their destructors
    // (which may run later, after the session is gone) are safe no-ops.
    _cbAvatarLoaded.Unregister();
    _cbGameJoinRequested.Unregister();
    // The session has already settled pending requests; release backend copies.
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

void FriendsSteam::getFriends(OnFriends callback) {
    auto* friends = SteamFriends();
    if (!friends) { callback.failure({GsErrorCode::NotReady, "Friends unavailable"}); return; }
    const int count = friends->GetFriendCount(k_EFriendFlagImmediate);
    if (count < 0) { callback.failure({GsErrorCode::NotReady, "Friend list unavailable"}); return; }
    std::vector<FriendInfo> result;
    for (int i = 0; i < count; ++i) {
        const auto id = friends->GetFriendByIndex(i, k_EFriendFlagImmediate);
        const char* name = friends->GetFriendPersonaName(id);
        if (!id.IsValid() || !name) {
            callback.failure({GsErrorCode::PlatformError, "Friend lookup failed"});
            return;
        }
        FriendInfo info;
        info.userId = std::to_string(id.ConvertToUint64());
        info.displayName = name;
        const char* nick = friends->GetPlayerNickname(id);
        if (nick && *nick) info.nickname = nick;
        info.presence = toPresence(friends->GetFriendPersonaState(id));
        result.push_back(std::move(info));
    }
    callback.success(std::move(result));
}

AvatarImage FriendsSteam::fetchAvatar(int handle) {
    AvatarImage img;
    if (handle <= 0 || !SteamUtils()) return img;

    uint32 w = 0, h = 0;
    if (!SteamUtils()->GetImageSize(handle, &w, &h) || w == 0 || h == 0) return img;

    const uint64_t pixels = static_cast<uint64_t>(w) * h;
    if (pixels > static_cast<uint64_t>(std::numeric_limits<int>::max()) / 4) return img;
    const uint64_t byteCount = pixels * 4;
    img.width = static_cast<int>(w);
    img.height = static_cast<int>(h);
    img.data.resize(static_cast<size_t>(byteCount));
    if (!SteamUtils()->GetImageRGBA(handle, img.data.data(), static_cast<int>(img.data.size()))) {
        img.data.clear();
        img.width = 0;
        img.height = 0;
    }
    return img;
}

void FriendsSteam::getAvatar(const AccountId& userId, AvatarSize size, OnAvatarLoaded callback) {
    if (!SteamFriends() || !SteamUtils()) { callback.failure({GsErrorCode::NotReady, "Avatar service unavailable"}); return; }
    if (size != AvatarSize::Small && size != AvatarSize::Medium && size != AvatarSize::Large) {
        callback.failure({GsErrorCode::InvalidArgument, "Invalid avatar size"}); return;
    }
    CSteamID id;
    if (!parseSteamId(userId, id)) {
        callback.failure({GsErrorCode::InvalidArgument, "Invalid userId"});
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
        if (img.data.empty()) callback.failure({GsErrorCode::PlatformError, "Avatar image read failed"});
        else callback.success(img);
    } else if (size == AvatarSize::Large && handle == -1) {
        // Prevent memory leak: cap the pending queue to avoid infinite accumulation
        // if the Steam network fails to trigger AvatarImageLoaded_t.
        update(); // Reclaim timed-out entries before enforcing the queue limit.
        if (_pendingAvatars.size() >= 50) {
            callback.failure({GsErrorCode::Busy, "Avatar request queue full"});
            return;
        }
        _pendingAvatars.push_back({id, size, std::move(callback)});
    } else {
        callback.success(std::nullopt);
    }
}

void FriendsSteam::onAvatarImageLoaded(AvatarImageLoaded_t* pParam) {
    if (!pParam) return;
    std::vector<PendingAvatar> completed;
    for (auto it = _pendingAvatars.begin(); it != _pendingAvatars.end(); ) {
        if (it->steamId == pParam->m_steamID) {
            completed.push_back(std::move(*it));
            it = _pendingAvatars.erase(it);
        } else {
            ++it;
        }
    }
    // User callbacks can enqueue more requests. Never hold a queue iterator
    // while invoking them, and do not consume newly enqueued requests here.
    for (auto& request : completed) {
        if (!request.callback) continue;
        int handle = 0;
        switch (request.size) {
        case AvatarSize::Small:  handle = SteamFriends()->GetSmallFriendAvatar(request.steamId); break;
        case AvatarSize::Medium: handle = SteamFriends()->GetMediumFriendAvatar(request.steamId); break;
        case AvatarSize::Large:  handle = SteamFriends()->GetLargeFriendAvatar(request.steamId); break;
        }
        if (handle > 0) {
            AvatarImage img = fetchAvatar(handle);
            if (img.data.empty()) request.callback.failure({GsErrorCode::PlatformError, "Avatar image read failed"});
            else request.callback.success(img);
        } else if (handle == 0) {
            request.callback.success(std::nullopt);
        } else {
            request.callback.failure({GsErrorCode::PlatformError, "Avatar load failed"});
        }
    }
}

void FriendsSteam::update() {
    // Session owns timeout settlement. This backend only releases finished waiters.
    _pendingAvatars.erase(std::remove_if(_pendingAvatars.begin(), _pendingAvatars.end(),
        [](const auto& request) { return !request.callback; }), _pendingAvatars.end());
}

void FriendsSteam::getGroups(OnFriendGroups callback) {
    auto* friends = SteamFriends();
    if (!friends) { callback.failure({GsErrorCode::NotReady, "Friends unavailable"}); return; }
    std::vector<FriendGroup> result;
    const int groupCount = friends->GetFriendsGroupCount();
    if (groupCount < 0) { callback.failure({GsErrorCode::PlatformError, "Group lookup failed"}); return; }
    for (int i = 0; i < groupCount; ++i) {
        const auto gid = friends->GetFriendsGroupIDByIndex(i);
        if (gid == k_FriendsGroupID_Invalid) {
            callback.failure({GsErrorCode::PlatformError, "Invalid friend group"}); return;
        }
        FriendGroup group;
        group.id = std::to_string(gid);
        const char* name = friends->GetFriendsGroupName(gid);
        group.displayName = name ? name : "";
        const int count = friends->GetFriendsGroupMembersCount(gid);
        if (count < 0) { callback.failure({GsErrorCode::PlatformError, "Group members unavailable"}); return; }
        std::vector<CSteamID> members(count);
        if (count) friends->GetFriendsGroupMembersList(gid, members.data(), count);
        for (const auto& id : members) {
            if (!id.IsValid()) { callback.failure({GsErrorCode::PlatformError, "Invalid group member"}); return; }
            group.memberIds.push_back(std::to_string(id.ConvertToUint64()));
        }
        result.push_back(std::move(group));
    }
    callback.success(std::move(result));
}

void FriendsSteam::setRichPresence(const std::string& key, const std::string& value, OnComplete callback) {
    if (!validText(key) || value.find('\0') != std::string::npos) {
        callback.failure({GsErrorCode::InvalidArgument, "Invalid presence key or value"}); return;
    }
    if (!SteamFriends()) { callback.failure({GsErrorCode::NotReady, "Friends unavailable"}); return; }
    if (!SteamFriends()->SetRichPresence(key.c_str(), value.c_str())) {
        callback.failure({GsErrorCode::PlatformError, "Presence update rejected"}); return;
    }
    callback.success();
}

void FriendsSteam::clearRichPresence(OnComplete callback) {
    if (!SteamFriends()) { callback.failure({GsErrorCode::NotReady, "Friends unavailable"}); return; }
    SteamFriends()->ClearRichPresence();
    callback.success();
}

void FriendsSteam::getRichPresence(const AccountId& userId, const std::string& key, OnPresenceValue callback) {
    CSteamID id;
    if (!parseSteamId(userId, id) || !validText(key)) {
        callback.failure({GsErrorCode::InvalidArgument, "Invalid userId or presence key"}); return;
    }
    if (!SteamFriends()) { callback.failure({GsErrorCode::NotReady, "Friends unavailable"}); return; }
    // Steam exposes the currently cached value; this is not a network refresh.
    const char* value = SteamFriends()->GetFriendRichPresence(id, key.c_str());
    if (value && *value) callback.success({std::string(value)});
    else callback.success({std::nullopt});
}

void FriendsSteam::openOverlay(OverlayDialog dialog, OnComplete callback) {
    if (!SteamFriends() || !SteamUtils()) { callback.failure({GsErrorCode::NotReady, "Overlay unavailable"}); return; }
    const char* name = toSteamOverlayDialog(dialog);
    if (!name) { callback.failure({GsErrorCode::InvalidArgument, "Invalid overlay page"}); return; }
    if (!SteamUtils()->IsOverlayEnabled()) {
        callback.failure({GsErrorCode::NotReady, "Steam overlay is not enabled"}); return;
    }
    SteamFriends()->ActivateGameOverlay(name);
    callback.success(); // Request dispatched; no visibility confirmation is available.
}

void FriendsSteam::openWebPage(const std::string& url, OnComplete callback) {
    if (!validText(url) || (url.rfind("https://", 0) != 0 && url.rfind("http://", 0) != 0)) {
        callback.failure({GsErrorCode::InvalidArgument, "Expected an HTTP or HTTPS URL"}); return;
    }
    if (!SteamFriends() || !SteamUtils() || !SteamUtils()->IsOverlayEnabled()) {
        callback.failure({GsErrorCode::NotReady, "Steam overlay is not enabled"}); return;
    }
    SteamFriends()->ActivateGameOverlayToWebPage(url.c_str());
    callback.success();
}

void FriendsSteam::setOnJoinRequested(OnJoinRequested delegate) {
    _gameRichPresenceJoinDelegate = std::move(delegate);
}

void FriendsSteam::onGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t* pParam) {
    if (pParam && _gameRichPresenceJoinDelegate) {
        std::string friendId = std::to_string(pParam->m_steamIDFriend.ConvertToUint64());
        std::string connectStr = pParam->m_rgchConnect;
        _gameRichPresenceJoinDelegate.invoke({friendId, connectStr});
    }
}

} // namespace cc::Gs

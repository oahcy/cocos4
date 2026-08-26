#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "commons/GsCallback.h"
#include "commons/GsTypes.h"
#include "base/RefCounted.h"

namespace cc::Gs {

// Values must stay in sync with Steam's EPersonaState: FriendsSteam casts the
// Steam value straight across.
enum class PersonaState : uint8_t {
    Offline = 0,
    Online,
    Busy,
    Away,
    Snooze,
    LookingToTrade,
    LookingToPlay,
    Invisible
};

enum class AvatarSize : uint8_t {
    Small = 0,
    Medium,
    Large
};

// Bit flags for getFriends(). Each platform maps these onto its own constants.
enum class FriendFlags : uint32_t {
    None = 0,
    Immediate = 1U << 0,            // confirmed friends
    Blocked = 1U << 1,
    FriendshipRequested = 1U << 2,  // incoming request
    RequestingFriendship = 1U << 3, // outgoing request
    ClanMember = 1U << 4,
    OnGameServer = 1U << 5,
    All = 0xFFFFFFFFU
};

inline FriendFlags operator|(FriendFlags a, FriendFlags b) {
    return static_cast<FriendFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline bool hasFlag(FriendFlags value, FriendFlags flag) {
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

// Platform overlay pages. Not every platform supports every page; unsupported
// values are rejected by the implementation.
enum class OverlayDialog : uint8_t {
    Friends = 0,
    Community,
    Players,
    Settings,
    OfficialGameGroup,
    Stats,
    Achievements
};

struct FriendInfo {
    AccountId userId;
    std::string personaName;
    std::string nickname;
    PersonaState personaState = PersonaState::Offline;
};

struct FriendListResult {
    std::vector<FriendInfo> Friends;
};

struct AvatarImage {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> data;
};

struct FriendsGroupInfo {
    int16_t groupId = -1;
    std::string groupName;
    std::vector<AccountId> members;
};

struct FriendsGroupListResult {
    std::vector<FriendsGroupInfo> Groups;
};

#ifdef SWIG
using OnAvatarLoaded = AsyncCallbackBase;
#else
using OnAvatarLoaded = AsyncCallback<AvatarImage>;
#endif

class IFriends : public cc::RefCounted {
public:
    virtual ~IFriends() = default;

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

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include "commons/GsCallback.h"
#include "commons/GsTypes.h"
#include "base/RefCounted.h"
#include "base/Ptr.h"

namespace cc::Gs {
class GsSession;

// Platform values are explicitly mapped by each backend.
enum class PresenceState : uint8_t { Unknown = 0, Offline, Online, Away, Busy };
enum class AvatarSize : uint8_t { Small = 0, Medium, Large };
enum class OverlayDialog : uint8_t {
    Friends = 0, Community, Players, Settings, OfficialGameGroup, Stats, Achievements
};

struct FriendInfo {
    AccountId userId;
    std::string displayName;
    std::optional<std::string> nickname;
    PresenceState presence = PresenceState::Unknown;
};
struct AvatarImage {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> data; // RGBA8, tightly packed rows.
};
struct FriendGroup {
    std::string id;
    std::string displayName;
    std::vector<AccountId> memberIds;
};
struct PresenceValue { std::optional<std::string> value; };
struct JoinRequest {
    AccountId userId;
    std::string connectionString; // Opaque game-defined connection data.
};

#ifdef SWIG
using OnFriends = AsyncCallbackBase;
using OnAvatarLoaded = AsyncCallbackBase;
using OnFriendGroups = AsyncCallbackBase;
using OnPresenceValue = AsyncCallbackBase;
using OnJoinRequested = EventDelegateBase;
#else
using OnFriends = AsyncCallback<std::vector<FriendInfo>>;
using OnAvatarLoaded = AsyncCallback<std::optional<AvatarImage>>;
using OnFriendGroups = AsyncCallback<std::vector<FriendGroup>>;
using OnPresenceValue = AsyncCallback<PresenceValue>;
using OnJoinRequested = EventDelegate<JoinRequest>;
#endif

// JSB facade retains its original session, never the backend.
class IFriends final : public cc::RefCounted {
public:
    ~IFriends() override;
    void getFriends(OnFriends callback);
    void getAvatar(const AccountId& userId, AvatarSize size, OnAvatarLoaded callback);
    void getGroups(OnFriendGroups callback);
    void setRichPresence(const std::string& key, const std::string& value, OnComplete callback);
    void clearRichPresence(OnComplete callback);
    void getRichPresence(const AccountId& userId, const std::string& key, OnPresenceValue callback);
    void openOverlay(OverlayDialog dialog, OnComplete callback);
    void openWebPage(const std::string& url, OnComplete callback);
    void setOnJoinRequested(OnJoinRequested delegate);
#ifndef SWIG
    explicit IFriends(cc::IntrusivePtr<GsSession> session);
private:
    cc::IntrusivePtr<GsSession> _session;
#endif
};
} // namespace cc::Gs

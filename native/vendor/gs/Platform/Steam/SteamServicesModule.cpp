#include "SteamServicesModule.h"
#include "AchievementsSteam.h"
#include "FriendsSteam.h"
#include "RemoteStorageSteam.h"
#include "StatsSteam.h"
#include "UtilsSteam.h"
#include "base/Log.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace cc::Gs {

bool SteamPlatform::checkDllAvailable() {
#ifdef _WIN32
    if (!_dllChecked) {
        _dllChecked = true;
        HMODULE h = LoadLibraryA("steam_api64.dll");
        if (h) {
            FreeLibrary(h);
        } else {
            _dllAvailable = false;
            CC_LOG_ERROR("[Steam] steam_api64.dll not found");
        }
    }
#endif
    return _dllAvailable;
}

void SteamPlatform::restartAppIfNecessary(const AppId& appId, OnRestartRequired callback) {
    const auto* numericAppId = ccstd::get_if<uint32_t>(&appId);
    if (!numericAppId || *numericAppId == 0) {
        callback.failure({GsErrorCode::InvalidArgument, "Steam requires a nonzero numeric App ID"}); return;
    }
    if (!checkDllAvailable()) { callback.failure({GsErrorCode::NotReady, "steam_api64.dll is unavailable"}); return; }
    callback.success(SteamAPI_RestartAppIfNecessary(*numericAppId));
}

std::optional<GsError> SteamPlatform::initialize(GsModules& modules) {
    if (!checkDllAvailable()) return GsError{GsErrorCode::NotReady, "steam_api64.dll is unavailable"};
    SteamErrMsg error = {};
    const auto result = SteamAPI_InitEx(&error);
    if (result != k_ESteamAPIInitResult_OK) {
        CC_LOG_ERROR("[Steam] SteamAPI_Init failed: %s", error);
        return GsError{GsErrorCode::PlatformError, error, std::to_string(static_cast<int>(result))};
    }
    modules.achievements = std::make_unique<AchievementsSteam>();
    modules.friends = std::make_unique<FriendsSteam>();
    _friends = static_cast<FriendsSteam*>(modules.friends.get());
    modules.remoteStorage = std::make_unique<RemoteStorageSteam>();
    modules.stats = std::make_unique<StatsSteam>();
    modules.utils = std::make_unique<UtilsSteam>();
    CC_LOG_INFO("[Steam] SteamAPI_Init OK");
    return std::nullopt;
}

void SteamPlatform::pump(float) {
    // Session Dispatch keeps backends alive throughout this pump.
    _friends->expireAvatarRequests();
    SteamAPI_RunCallbacks();
}
void SteamPlatform::shutdown() {
    _friends = nullptr;
    SteamAPI_Shutdown();
    CC_LOG_INFO("[Steam] SteamAPI_Shutdown");
}

IntrusivePtr<IGsServices> createSteamServices() {
    return new GsServicesCommon(GsServicesType::Steam, std::make_unique<SteamPlatform>());
}

} // namespace cc::Gs

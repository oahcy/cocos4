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

bool SteamPlatform::restartAppIfNecessary(const AppId& appId) {
    if (!checkDllAvailable()) return false;
    if (auto* numericAppId = ccstd::get_if<uint32_t>(&appId)) {
        return SteamAPI_RestartAppIfNecessary(*numericAppId);
    }
    CC_LOG_ERROR("[Steam] restartAppIfNecessary: string AppId is not supported on Steam (requires a numeric App ID)");
    return false;
}

bool SteamPlatform::initialize(GsModules& modules) {
    if (!checkDllAvailable()) return false;
    SteamErrMsg error = {};
    if (SteamAPI_InitEx(&error) != k_ESteamAPIInitResult_OK) {
        CC_LOG_ERROR("[Steam] SteamAPI_Init failed: %s", error);
        return false;
    }
    modules.achievements = std::make_unique<AchievementsSteam>();
    modules.friends = std::make_unique<FriendsSteam>();
    _friends = static_cast<FriendsSteam*>(modules.friends.get());
    modules.remoteStorage = std::make_unique<RemoteStorageSteam>();
    // Both backends belong to this session; the callback is only used by reset.
    auto* achievements = static_cast<AchievementsSteam*>(modules.achievements.get());
    modules.stats = std::make_unique<StatsSteam>([achievements] { achievements->invalidateStates(); });
    modules.utils = std::make_unique<UtilsSteam>();
    CC_LOG_INFO("[Steam] SteamAPI_Init OK");
    return true;
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

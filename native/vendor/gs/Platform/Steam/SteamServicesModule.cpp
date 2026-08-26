#include "SteamServicesModule.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace cc::Gs {

bool SteamGsServices::checkDllAvailable() {
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

bool SteamGsServices::restartAppIfNecessary(const AppId& appId) {
    if (!checkDllAvailable()) return false;
    if (auto* numericAppId = ccstd::get_if<uint32_t>(&appId)) {
        return SteamAPI_RestartAppIfNecessary(*numericAppId);
    }
    CC_LOG_ERROR("[Steam] restartAppIfNecessary: string AppId is not supported on Steam (requires a numeric App ID)");
    return false;
}

} // namespace cc::Gs

#include "AccountSteam.h"
#include <steam_api.h>

namespace cc::Gs {
void AccountSteam::getUser(OnAccountUser callback) {
    if (!SteamUser() || !SteamFriends()) {
        callback.failure({GsErrorCode::NotReady, "Account unavailable"});
        return;
    }
    const auto id = SteamUser()->GetSteamID();
    if (!id.IsValid()) { callback.success(std::nullopt); return; }
    const char* name = SteamFriends()->GetPersonaName();
    if (!name) {
        callback.failure({GsErrorCode::PlatformError, "Account name lookup failed"});
        return;
    }
    // A local Steam identity can remain available in offline mode.
    callback.success(UserProfile{std::to_string(id.ConvertToUint64()), name});
}
} // namespace cc::Gs

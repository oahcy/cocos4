#include "UtilsSteam.h"

#include "base/Log.h"

namespace cc::Gs {

OnWarningMessage UtilsSteam::s_callback;

void UtilsSteam::steamWarningHook(int severity, const char* msg) {
    CC_LOG_INFO("[Steam Warning] severity=%d: %s", severity, msg);
    if (s_callback) {
        s_callback.invoke(severity, std::string(msg));
    }
}

void UtilsSteam::shutdown() {
    // Release the rooted JS warning hook while Steam is still alive, so the
    // static listener is not leaked past the session.
    s_callback.reset();
    auto* utils = SteamUtils();
    if (utils) {
        utils->SetWarningMessageHook(nullptr);
    }
    Super::shutdown();
}

void UtilsSteam::setWarningMessageHook(OnWarningMessage callback) {
    if (isShutdown()) return;
    s_callback = std::move(callback);
    auto* utils = SteamUtils();
    if (utils) {
        if (s_callback) {
            utils->SetWarningMessageHook(&UtilsSteam::steamWarningHook);
            CC_LOG_INFO("[Steam] WarningMessageHook set");
        } else {
            utils->SetWarningMessageHook(nullptr);
            CC_LOG_INFO("[Steam] WarningMessageHook cleared");
        }
    }
}

} // namespace cc::Gs

#include "UtilsSteam.h"

#include "base/Log.h"

namespace cc::Gs {

OnDiagnostic UtilsSteam::s_callback;

void UtilsSteam::steamWarningHook(int severity, const char* msg) {
    CC_LOG_INFO("[Steam Warning] severity=%d: %s", severity, msg ? msg : "");
    if (s_callback) {
        const auto level = severity == 0 ? DiagnosticLevel::Info
            : (severity == 1 ? DiagnosticLevel::Warning : DiagnosticLevel::Unknown);
        s_callback.invoke({level, msg ? msg : ""});
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
}

void UtilsSteam::setOnDiagnostic(OnDiagnostic callback) {
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

#pragma once

#include <steam_api.h>
#include "../../Framework/backends/UtilsBackend.h"

namespace cc::Gs {

class UtilsSteam : public IUtilsBackend {
public:

    void shutdown() override;

    void setOnDiagnostic(OnDiagnostic callback) override;

private:
    static void steamWarningHook(int severity, const char* msg);

    static OnDiagnostic s_callback;
};

} // namespace cc::Gs

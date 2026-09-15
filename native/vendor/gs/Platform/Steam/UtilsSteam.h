#pragma once

#include <steam_api.h>
#include "../../Framework/backends/UtilsBackend.h"

namespace cc::Gs {

class UtilsSteam : public IUtilsBackend {
public:

    void shutdown() override;

    void setWarningMessageHook(OnWarningMessage callback) override;

private:
    static void steamWarningHook(int severity, const char* msg);

    static OnWarningMessage s_callback;
};

} // namespace cc::Gs

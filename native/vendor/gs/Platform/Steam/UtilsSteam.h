#pragma once

#include <steam_api.h>
#include "../../Framework/Utils.h"
#include "../../Framework/commons/GsComponent.h"

namespace cc::Gs {

class UtilsSteam : public GsComponent<IUtils> {
public:
    using Super = GsComponent<IUtils>;

    explicit UtilsSteam(GsServicesCommon& inServices)
        : Super(inServices) {}

    void shutdown() override;

    void setWarningMessageHook(OnWarningMessage callback) override;

private:
    static void steamWarningHook(int severity, const char* msg);

    static OnWarningMessage s_callback;
};

} // namespace cc::Gs

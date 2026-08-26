#pragma once

#include <steam_api.h>
#include "../../Framework/Stats.h"
#include "../../Framework/commons/GsComponent.h"

namespace cc::Gs {

class StatsSteam : public GsComponent<IStats> {
public:
    using Super = GsComponent<IStats>;

    explicit StatsSteam(GsServicesCommon& inServices)
        : Super(inServices) {}

    void shutdown() override;

    void setStatInt(const std::string& name, int32_t value, OnComplete callback) override;
    void setStatFloat(const std::string& name, float value, OnComplete callback) override;
    StatIntResult getStatInt(const std::string& name) override;
    StatFloatResult getStatFloat(const std::string& name) override;
    void storeStats(OnComplete callback) override;
    bool resetAllStats(bool achievementsToo) override;
};

} // namespace cc::Gs

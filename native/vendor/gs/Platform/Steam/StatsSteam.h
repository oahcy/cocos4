#pragma once

#include <steam_api.h>
#include <functional>
#include "../../Framework/backends/StatsBackend.h"

namespace cc::Gs {

class StatsSteam : public IStatsBackend {
public:
    explicit StatsSteam(std::function<void()> invalidateAchievements = {})
        : _invalidateAchievements(std::move(invalidateAchievements)) {}

    void setStatInt(const std::string& name, int32_t value, OnComplete callback) override;
    void setStatFloat(const std::string& name, float value, OnComplete callback) override;
    StatIntResult getStatInt(const std::string& name) override;
    StatFloatResult getStatFloat(const std::string& name) override;
    void storeStats(OnComplete callback) override;
    bool resetAllStats(bool achievementsToo) override;
private:
    std::function<void()> _invalidateAchievements;
};

} // namespace cc::Gs

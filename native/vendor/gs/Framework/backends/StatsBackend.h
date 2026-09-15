#pragma once

#include "../Stats.h"
#include "GsBackend.h"

namespace cc::Gs {

class IStatsBackend : public GsBackend {
public:
    virtual void setStatInt(const std::string& name, int32_t value, OnComplete callback) = 0;
    virtual void setStatFloat(const std::string& name, float value, OnComplete callback) = 0;
    virtual StatIntResult getStatInt(const std::string& name) = 0;
    virtual StatFloatResult getStatFloat(const std::string& name) = 0;
    virtual void storeStats(OnComplete callback) = 0;
    virtual bool resetAllStats(bool achievementsToo) = 0;
};

} // namespace cc::Gs

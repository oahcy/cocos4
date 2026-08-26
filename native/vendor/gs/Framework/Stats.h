#pragma once

#include <string>
#include <cstdint>
#include "commons/GsCallback.h"
#include "base/RefCounted.h"

namespace cc::Gs {

struct StatIntResult {
    bool Success = false;
    int32_t Value = 0;
};

struct StatFloatResult {
    bool Success = false;
    float Value = 0.0f;
};

class IStats : public cc::RefCounted {
public:
    virtual ~IStats() = default;

    virtual void setStatInt(const std::string& name, int32_t value, OnComplete callback) = 0;
    virtual void setStatFloat(const std::string& name, float value, OnComplete callback) = 0;
    virtual StatIntResult getStatInt(const std::string& name) = 0;
    virtual StatFloatResult getStatFloat(const std::string& name) = 0;
    virtual void storeStats(OnComplete callback) = 0;
    virtual bool resetAllStats(bool achievementsToo) = 0;
};

} // namespace cc::Gs

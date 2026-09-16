#pragma once
#include "../Stats.h"
#include "GsBackend.h"

namespace cc::Gs {
class IStatsBackend : public GsBackend {
public:
    virtual void getInt(const std::string& name, OnStatInt callback) = 0;
    virtual void getFloat(const std::string& name, OnStatFloat callback) = 0;
    virtual void setInt(const std::string& name, int64_t value, OnComplete callback) { callback.failure({GsErrorCode::NotSupported, "setInt is not supported"}); }
    virtual void setFloat(const std::string& name, double value, OnComplete callback) { callback.failure({GsErrorCode::NotSupported, "setFloat is not supported"}); }
    virtual void incrementInt(const std::string& name, int64_t delta, OnComplete callback) { callback.failure({GsErrorCode::NotSupported, "incrementInt is not supported"}); }
    virtual void incrementFloat(const std::string& name, double delta, OnComplete callback) { callback.failure({GsErrorCode::NotSupported, "incrementFloat is not supported"}); }
    virtual void flush(OnComplete callback) { callback.failure({GsErrorCode::NotSupported, "flush is not supported"}); }
    virtual void resetAll(bool includeAchievements, OnComplete callback) { callback.failure({GsErrorCode::NotSupported, "resetAll is not supported"}); }
};
} // namespace cc::Gs

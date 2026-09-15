#pragma once

#include <string>
#include <cstdint>
#include "commons/GsCallback.h"
#include "base/RefCounted.h"
#include "base/Ptr.h"

namespace cc::Gs {

class GsSession;

struct StatIntResult {
    bool Success = false;
    int32_t Value = 0;
};

struct StatFloatResult {
    bool Success = false;
    float Value = 0.0f;
};

// JSB facade: retains its original session, never the platform implementation.
class IStats final : public cc::RefCounted {
public:
    ~IStats() override;
    void setStatInt(const std::string& name, int32_t value, OnComplete callback);
    void setStatFloat(const std::string& name, float value, OnComplete callback);
    StatIntResult getStatInt(const std::string& name);
    StatFloatResult getStatFloat(const std::string& name);
    void storeStats(OnComplete callback);
    bool resetAllStats(bool achievementsToo);
#ifndef SWIG
    explicit IStats(cc::IntrusivePtr<GsSession> session);
private:
    cc::IntrusivePtr<GsSession> _session;
#endif
};

} // namespace cc::Gs

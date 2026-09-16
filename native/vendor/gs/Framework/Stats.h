#pragma once
#include <string>
#include <cstdint>
#include "commons/GsCallback.h"
#include "base/RefCounted.h"
#include "base/Ptr.h"

namespace cc::Gs {
class GsSession;
#ifdef SWIG
using OnStatInt = AsyncCallbackBase;
using OnStatFloat = AsyncCallbackBase;
#else
// Integer results use the JS Number transport; backends must return safe integers.
// int64_t would be converted to a JS BigInt by the shared binding converters.
using OnStatInt = AsyncCallback<double>;
using OnStatFloat = AsyncCallback<double>;
#endif

// JSB facade retains its original session, never the platform backend.
class IStats final : public cc::RefCounted {
public:
    ~IStats() override;
    void getInt(const std::string& name, OnStatInt callback);
    void getFloat(const std::string& name, OnStatFloat callback);
    void setInt(const std::string& name, int64_t value, OnComplete callback);
    void setFloat(const std::string& name, double value, OnComplete callback);
    void incrementInt(const std::string& name, int64_t delta, OnComplete callback);
    void incrementFloat(const std::string& name, double delta, OnComplete callback);
    void flush(OnComplete callback);
    void resetAll(bool includeAchievements, OnComplete callback);
#ifndef SWIG
    explicit IStats(cc::IntrusivePtr<GsSession> session);
private:
    cc::IntrusivePtr<GsSession> _session;
#endif
};
} // namespace cc::Gs

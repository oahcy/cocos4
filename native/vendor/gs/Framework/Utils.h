#pragma once

#include "commons/GsCallback.h"
#include "base/RefCounted.h"
#include "base/Ptr.h"

namespace cc::Gs {

class GsSession;

enum class DiagnosticLevel : uint8_t { Unknown = 0, Info, Warning, Error };
struct DiagnosticMessage {
    DiagnosticLevel level = DiagnosticLevel::Unknown;
    std::string message;
};
#ifdef SWIG
using OnDiagnostic = EventDelegateBase;
#else
using OnDiagnostic = EventDelegate<DiagnosticMessage>;
#endif


// JSB facade: retains its original session, never the platform implementation.
class IUtils final : public cc::RefCounted {
public:
    ~IUtils() override;
    void setOnDiagnostic(OnDiagnostic callback);
#ifndef SWIG
    explicit IUtils(cc::IntrusivePtr<GsSession> session);
private:
    cc::IntrusivePtr<GsSession> _session;
#endif
};

} // namespace cc::Gs

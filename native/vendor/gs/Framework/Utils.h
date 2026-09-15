#pragma once

#include "commons/GsCallback.h"
#include "base/RefCounted.h"
#include "base/Ptr.h"

namespace cc::Gs {

class GsSession;

// JSB facade: retains its original session, never the platform implementation.
class IUtils final : public cc::RefCounted {
public:
    ~IUtils() override;
    void setWarningMessageHook(OnWarningMessage callback);
#ifndef SWIG
    explicit IUtils(cc::IntrusivePtr<GsSession> session);
private:
    cc::IntrusivePtr<GsSession> _session;
#endif
};

} // namespace cc::Gs

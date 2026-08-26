#pragma once

#include "commons/GsCallback.h"
#include "base/RefCounted.h"

namespace cc::Gs {

class IUtils : public cc::RefCounted {
public:
    virtual ~IUtils() = default;

    virtual void setWarningMessageHook(OnWarningMessage callback) = 0;
};

} // namespace cc::Gs

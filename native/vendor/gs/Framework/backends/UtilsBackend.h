#pragma once

#include "../Utils.h"
#include "GsBackend.h"

namespace cc::Gs {

class IUtilsBackend : public GsBackend {
public:
    virtual void setWarningMessageHook(OnWarningMessage callback) = 0;
};

} // namespace cc::Gs

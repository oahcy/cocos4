#pragma once

#include "../Utils.h"
#include "GsBackend.h"

namespace cc::Gs {

class IUtilsBackend : public GsBackend {
public:
    virtual void setOnDiagnostic(OnDiagnostic callback) = 0;
};

} // namespace cc::Gs

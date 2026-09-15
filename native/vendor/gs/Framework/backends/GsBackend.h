#pragma once

namespace cc::Gs {

// Implementations are session-owned, not ref-counted or exposed through JSB.
class GsBackend {
public:
    virtual ~GsBackend() = default;
    virtual void shutdown() {}
};

} // namespace cc::Gs

/****************************************************************************
 Copyright (c) 2025 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
****************************************************************************/

#pragma once

#include "GsComponentRegistry.h"

#include "base/Ptr.h"

namespace cc::Gs {

class GsServicesCommon;

// --- IGsComponent lifecycle interface ---

class IGsComponent {
public:
    virtual ~IGsComponent() = default;
    virtual void initialize() {}
    virtual void postInitialize() {}
    virtual void tick(float deltaTime) {}
    virtual void shutdown() {}
};

// --- GsComponent<ComponentType> CRTP mixin ---

template <typename ComponentType>
class GsComponent : public ComponentType, public IGsComponent {
public:
    using Super = ComponentType;

    // Explicit (empty) destructor so every component type gets a stable
    // breakpoint that fires when the component is actually released.
    ~GsComponent() override = default;

    explicit GsComponent(GsServicesCommon& inServices);

    GsServicesCommon& getServices() { return _services; }
    const GsServicesCommon& getServices() const { return _services; }

    // Marks the component shut down. Components override shutdown() to release
    // platform resources (Steam callbacks, JS listeners, pending calls) and must
    // call Super::shutdown() so this flag is set. Methods called afterwards fail
    // fast instead of touching a torn-down session.
    void shutdown() override { _shutdown = true; }

    bool isShutdown() const { return _shutdown; }

protected:
    GsServicesCommon& _services;
    bool _shutdown = false;
};

} // namespace cc::Gs

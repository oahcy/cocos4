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

#include <functional>
#include <unordered_map>
#include <vector>
#include "GsServices.h"

namespace cc::Gs {

class GsServicesRegistry {
public:
    using Factory = std::function<IntrusivePtr<IGsServices>()>;
    static GsServicesRegistry& get() {
        static GsServicesRegistry registry;
        return registry;
    }
    void registerProvider(GsServicesType provider, Factory factory) {
        _factories[provider] = std::move(factory);
    }
    IntrusivePtr<IGsServices> getServicesInstance(GsServicesType provider) {
        if (_cleaningUp) return nullptr;
        auto found = _services.find(provider);
        if (found != _services.end()) {
            // Do not initialize a replacement SDK while an old callback is unwinding.
            if (found->second->isClosing()) return nullptr;
            if (!found->second->isClosed()) return found->second;
        }
        auto factory = _factories.find(provider);
        if (factory == _factories.end()) return nullptr;
        auto services = factory->second();
        if (services) _services[provider] = services;
        return services;
    }
    void tick(float dt) {
        // Callbacks may request other providers; never iterate a mutable map while pumping.
        std::vector<IntrusivePtr<IGsServices>> services;
        for (auto& entry : _services) services.push_back(entry.second);
        for (auto& service : services) service->tick(dt);
    }
    void destroyServices() {
        if (_cleaningUp) return;
        _cleaningUp = true;
        for (auto& entry : _services) entry.second->destroy();
        _services.clear();
        _cleaningUp = false;
    }
private:
    GsServicesRegistry() = default;
    std::unordered_map<GsServicesType, Factory> _factories;
    std::unordered_map<GsServicesType, IntrusivePtr<IGsServices>> _services;
    bool _cleaningUp = false;
};

} // namespace cc::Gs

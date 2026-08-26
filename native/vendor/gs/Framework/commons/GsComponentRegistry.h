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

#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <memory>
#include <utility>

#include "base/Ptr.h"
#include "base/RefCounted.h"

namespace cc::Gs {

class IGsComponent;

// Owns one component instance per service interface. A concrete component is
// registered under the interface it implements (e.g. AchievementsSteam under
// IAchievements), so interface lookups are a single map access and lifecycle
// iteration (initialize/tick/shutdown) is a plain walk over the values. All
// components are stored as RefCounted, the common base every interface shares;
// the static_asserts below guarantee the downcasts in getComponent()/visit()
// are always valid.
class GsComponentRegistry {
public:
    // Empty destructor: a breakpoint here fires when the registry container is
    // destroyed, which only happens when the owning services object is released.
    ~GsComponentRegistry() = default;

    template <typename Interface, typename Concrete, typename... Args>
    void registerComponent(Args&&... args) {
        static_assert(std::is_base_of_v<Interface, Concrete>,
                      "Concrete must derive from the interface it is registered under");
        static_assert(std::is_base_of_v<IGsComponent, Concrete>,
                      "Concrete must implement the IGsComponent lifecycle");
        _components[std::type_index(typeid(Interface))] =
            cc::IntrusivePtr<cc::RefCounted>(new Concrete(std::forward<Args>(args)...));
    }

    template <typename Interface>
    cc::IntrusivePtr<Interface> getComponent() const {
        auto it = _components.find(std::type_index(typeid(Interface)));
        if (it == _components.end()) return nullptr;
        return cc::IntrusivePtr<Interface>(dynamic_cast<Interface*>(it->second.get()));
    }

    template <typename Fn>
    void visit(Fn&& visitor) {
        for (auto& [typeId, comp] : _components) {
            visitor(dynamic_cast<IGsComponent*>(comp.get()));
        }
    }

    void clear() {
        _components.clear();
    }

private:
    std::unordered_map<std::type_index, cc::IntrusivePtr<cc::RefCounted>> _components;
};

} // namespace cc::Gs

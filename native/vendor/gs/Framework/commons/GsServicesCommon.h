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

#include <cstdint>
#include <memory>

#include "GsServices.h"
#include "GsComponentRegistry.h"
#include "GsComponent.h"
#include "GsServicesRegistry.h"
#include "engine/EngineEvents.h"

#include "base/Ptr.h"

namespace cc::Gs {

class GsServicesCommon : public IGsServices {
public:
    GsServicesCommon(std::string instanceName, std::string instanceConfigName)
        : _instanceName(std::move(instanceName))
        , _instanceConfigName(std::move(instanceConfigName)) {}

    // Out-of-line so debuggers get a stable breakpoint that fires when the
    // services object is actually released (see GsServicesCommon.cpp).
    virtual ~GsServicesCommon();

    // --- IGsServices ---

    bool init() override {
        if (_isInitialized) {
            return true;
        }
        if (!onPreInitialize()) {
            return false;
        }

        _isInitialized = true;
        registerComponents();
        initializeComponents();
        postInitializeComponents();
        // The tick listener is created only here (not as a constructed member),
        // so a failed init never leaves an unbound listener registered on the
        // Engine Tick bus spamming "has no listener found" every frame.
        _tickListener = std::make_unique<events::Tick::Listener>();
        _tickListener->bind([this](float dt) {
            this->tick(dt);
        });
        return true;
    }

    // Shuts the services down. Components are ref-counted: the registry drops its
    // references here, but a component the JS layer still holds stays alive until the
    // corresponding JS wrapper is garbage-collected, so post-destroy calls never touch
    // freed memory. Platform teardown (e.g. SteamAPI_Shutdown) runs immediately.
    void destroy() override {
        if (!_isInitialized) return;

        // Destroying the listener removes it from the Engine Tick bus, so no
        // "has no listener found" spam after teardown (unlike reset(), which
        // only clears the callback).
        _tickListener.reset();
        _components.visit([](IGsComponent* comp) { comp->shutdown(); });
        _components.clear();
        onPostShutdown();
        
        _isInitialized = false;
    }

    void tick(float deltaTime) override {
        _components.visit([deltaTime](IGsComponent* comp) { comp->tick(deltaTime); });
    }

    cc::IntrusivePtr<IAchievements> getAchievementsInterface() override;
    cc::IntrusivePtr<IFriends> getFriendsInterface() override;
    cc::IntrusivePtr<IRemoteStorage> getRemoteStorageInterface() override;
    cc::IntrusivePtr<IStats> getStatsInterface() override;
    cc::IntrusivePtr<IUtils> getUtilsInterface() override;

    // --- Component access ---

    template <typename T>
    IntrusivePtr<T> get() { return _components.getComponent<T>(); }

    GsComponentRegistry& getComponentRegistry() { return _components; }

protected:
    // One-time platform hooks, driven by the base refcount:
    //  - onPreInitialize: runs before components are registered/initialized (e.g. SteamAPI_Init).
    //    Return false to abort init; base state is left untouched.
    //  - onPostShutdown: runs after components are shut down (e.g. SteamAPI_Shutdown).
    virtual bool onPreInitialize() { return true; }
    virtual void onPostShutdown() {}

    virtual void registerComponents() {}

    void initializeComponents() {
        _components.visit([](IGsComponent* comp) { comp->initialize(); });
    }

    void postInitializeComponents() {
        _components.visit([](IGsComponent* comp) { comp->postInitialize(); });
    }

    std::string _instanceName;
    std::string _instanceConfigName;

    GsComponentRegistry _components;
    bool _isInitialized = false;
    // Delayed-created so it only exists (and is registered on the bus) while the
    // services are initialized. A failed init or a destroy leaves this null.
    std::unique_ptr<events::Tick::Listener> _tickListener;
};

// --- GsComponent deferred implementations ---

template <typename ComponentType>
GsComponent<ComponentType>::GsComponent(GsServicesCommon& inServices)
    : _services(inServices) {}

} // namespace cc::Gs

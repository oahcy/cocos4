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

#include <steam_api.h>
#include "../../Framework/commons/GsServicesCommon.h"
#include "../../Framework/commons/GsServicesRegistry.h"
#include "AchievementsSteam.h"
#include "FriendsSteam.h"
#include "RemoteStorageSteam.h"
#include "StatsSteam.h"
#include "UtilsSteam.h"

#include "base/Log.h"

namespace cc::Gs {

class SteamGsServices : public GsServicesCommon {
public:
    using Super = GsServicesCommon;

    SteamGsServices(std::string instanceName, std::string instanceConfigName)
        : GsServicesCommon(std::move(instanceName), std::move(instanceConfigName)) {}

    bool restartAppIfNecessary(const AppId& appId) override;

    // SteamAPI is brought up once before components initialize, and torn down
    // once after components shut down. The base class owns all refcounting.
    bool onPreInitialize() override {
        if (!checkDllAvailable()) return false;
        SteamErrMsg errMsg = { 0 };
        if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK) {
            CC_LOG_ERROR("[Steam] SteamAPI_Init failed: %s", errMsg);
            return false;
        }
        CC_LOG_INFO("[Steam] SteamAPI_Init OK, logged in as SteamID %llu", static_cast<unsigned long long>(SteamUser()->GetSteamID().ConvertToUint64()));
        return true;
    }

    void onPostShutdown() override {
        SteamAPI_Shutdown();
        CC_LOG_INFO("[Steam] SteamAPI_Shutdown");
    }

    void tick(float deltaTime) override {
        SteamAPI_RunCallbacks();
        Super::tick(deltaTime);
    }

    GsServicesType getServicesProvider() const override {
        return GsServicesType::Steam;
    }

protected:
    void registerComponents() override {
        CC_LOG_INFO("[SteamServices] registerComponents");
        getComponentRegistry().registerComponent<IAchievements, AchievementsSteam>(*this);
        getComponentRegistry().registerComponent<IFriends, FriendsSteam>(*this);
        getComponentRegistry().registerComponent<IRemoteStorage, RemoteStorageSteam>(*this);
        getComponentRegistry().registerComponent<IStats, StatsSteam>(*this);
        getComponentRegistry().registerComponent<IUtils, UtilsSteam>(*this);
        Super::registerComponents();
    }

private:
    bool _dllChecked = false;
    bool _dllAvailable = true;

    bool checkDllAvailable();
};

class SteamServicesFactory : public IGsServicesFactory {
public:
    cc::IntrusivePtr<IGsServices> create(std::string instanceName, std::string instanceConfigName) override {
        CC_LOG_INFO("[Factory] Creating SteamGsServices");
        return cc::IntrusivePtr<IGsServices>(new SteamGsServices(std::move(instanceName), std::move(instanceConfigName)));
    }
};

class SteamModuleInitializer : public IGsModuleInitializer {
public:
    void registerFactories(GsServicesRegistry& registry) override {
        CC_LOG_INFO("[Module] StartupModule: registering Steam factory");
        registry.registerServicesFactory(
            GsServicesType::Steam,
            std::make_unique<SteamServicesFactory>()
        );
    }
};

// Full module teardown: drop the factory and the cached instance together.
// The instance is only released from the registry here — NOT in destroy(),
// because destroy() must keep the same instance cached so a later
// getServices() returns the identical native and re-init works.
inline void shutdownSteamModule() {
    GsServicesRegistry::get().removeNamedServicesInstance(GsServicesType::Steam);
    GsServicesRegistry::get().unregisterServicesFactory(GsServicesType::Steam);
}

} // namespace cc::Gs

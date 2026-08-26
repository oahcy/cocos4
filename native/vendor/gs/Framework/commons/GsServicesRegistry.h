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

#include "GsServices.h"

#include <string>
#include <unordered_map>
#include <memory>

#include "base/Log.h"
#include "base/Ptr.h"

namespace cc::Gs {

// --- IGsModuleInitializer ---

class GsServicesRegistry;

class IGsModuleInitializer {
public:
    virtual ~IGsModuleInitializer() = default;
    virtual void registerFactories(GsServicesRegistry& registry) = 0;
};

// --- IGsServicesFactory ---

class IGsServicesFactory {
public:
    virtual ~IGsServicesFactory() = default;
    virtual cc::IntrusivePtr<IGsServices> create(std::string instanceName, std::string instanceConfigName) = 0;
};

// --- GsServicesRegistry singleton ---

class GsServicesRegistry {
public:
    static GsServicesRegistry& get() {
        static GsServicesRegistry instance;
        return instance;
    }

    void addModuleInitializer(GsServicesType type, IGsModuleInitializer* init) {
        _moduleInitializers[static_cast<size_t>(type)] = init;
    }

    void ensureInitialized() {
        for (size_t i = 0; i < static_cast<size_t>(GsServicesType::GsServicesType_Max); ++i) {
            if (_moduleInitializers[i] && !_initializedModules[i]) {
                _initializedModules[i] = true;
                _moduleInitializers[i]->registerFactories(*this);
            }
        }
    }

    void registerServicesFactory(
        GsServicesType servicesType,
        std::unique_ptr<IGsServicesFactory> factory)
    {
        _servicesFactories[servicesType] = std::move(factory);
        CC_LOG_INFO("[Registry] Registered factory for services type %d", (int)servicesType);
    }

    void unregisterServicesFactory(GsServicesType servicesType) {
        _servicesFactories.erase(servicesType);
    }

    cc::IntrusivePtr<IGsServices> getNamedServicesInstance(
        GsServicesType servicesType,
        const std::string& instanceName = "",
        const std::string& instanceConfigName = "")
    {
        auto& instanceMap = _namedServiceInstances[servicesType];
        std::string cacheKey = instanceName + "|" + instanceConfigName;
        auto it = instanceMap.find(cacheKey);
        if (it != instanceMap.end()) {
            return it->second;
        }

        // Platform SDKs (Steam, Epic, ...) tend to hold process-wide singleton
        // state under the hood, so a second differently-named instance of the
        // same GsServicesType would end up sharing/stomping the same underlying
        // session anyway. Reuse the existing instance instead of creating a
        // conflicting second one; this only restricts instances within the same
        // type, other types (e.g. Epic while Steam is active) are unaffected.
        if (!instanceMap.empty()) {
            CC_LOG_ERROR("[GsServicesRegistry] Services type %d already has an active instance; "
                         "creating multiple simultaneous instances of the same platform is not "
                         "supported. Reusing the existing instance instead of instanceName=\"%s\" instanceConfigName=\"%s\".",
                         static_cast<int>(servicesType), instanceName.c_str(), instanceConfigName.c_str());
            return instanceMap.begin()->second;
        }

        auto services = createServices(servicesType, instanceName, instanceConfigName);
        if (services) {
            instanceMap[cacheKey] = services;
        }
        return services;
    }

    void removeNamedServicesInstance(
        GsServicesType servicesType,
        const std::string& instanceName = "",
        const std::string& instanceConfigName = "")
    {
        auto outerIt = _namedServiceInstances.find(servicesType);
        if (outerIt == _namedServiceInstances.end()) return;
        std::string cacheKey = instanceName + "|" + instanceConfigName;
        outerIt->second.erase(cacheKey);
    }

private:
    GsServicesRegistry() = default;

    cc::IntrusivePtr<IGsServices> createServices(
        GsServicesType servicesType,
        const std::string& instanceName,
        const std::string& instanceConfigName)
    {
        auto it = _servicesFactories.find(servicesType);
        if (it == _servicesFactories.end()) {
            CC_LOG_ERROR("[Registry] No factory registered for services type %d", (int)servicesType);
            return nullptr;
        }

        CC_LOG_INFO("[Registry] Creating services instance for type %d", (int)servicesType);
        auto services = it->second->create(instanceName, instanceConfigName);
        return services;
    }

    std::unordered_map<GsServicesType, std::unique_ptr<IGsServicesFactory>> _servicesFactories;
    std::unordered_map<GsServicesType, std::unordered_map<std::string, cc::IntrusivePtr<IGsServices>>> _namedServiceInstances;

    IGsModuleInitializer* _moduleInitializers[static_cast<size_t>(GsServicesType::GsServicesType_Max)] = {};
    bool _initializedModules[static_cast<size_t>(GsServicesType::GsServicesType_Max)] = {};
};

} // namespace cc::Gs

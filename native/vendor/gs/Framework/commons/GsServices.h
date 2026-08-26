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
#include <string>
#include <memory>

#include "base/Ptr.h"
#include "base/RefCounted.h"
#include "GsTypes.h"

namespace cc::Gs {

class IAchievements;
class IFriends;
class IRemoteStorage;
class IStats;
class IUtils;

enum class GsServicesType : uint8_t {
    Null = 0,
    Steam = 1,
    Epic = 2,
    GsServicesType_Max
};

class IGsServices : public RefCounted {
public:
    virtual ~IGsServices() = default;

    virtual bool init() = 0;
    virtual void destroy() = 0;
    virtual void tick(float deltaTime) = 0;

    virtual GsServicesType getServicesProvider() const = 0;

    virtual cc::IntrusivePtr<IAchievements> getAchievementsInterface() = 0;
    virtual cc::IntrusivePtr<IFriends> getFriendsInterface() = 0;
    virtual cc::IntrusivePtr<IRemoteStorage> getRemoteStorageInterface() = 0;
    virtual cc::IntrusivePtr<IStats> getStatsInterface() = 0;
    virtual cc::IntrusivePtr<IUtils> getUtilsInterface() = 0;

    virtual bool restartAppIfNecessary(const AppId& /*appId*/) { return false; }

    static cc::IntrusivePtr<IGsServices> getServices(
        GsServicesType servicesType,
        const std::string& instanceName = "",
        const std::string& instanceConfigName = "");
};

} // namespace cc::Gs

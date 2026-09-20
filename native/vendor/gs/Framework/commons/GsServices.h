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
#include "GsCallback.h"

namespace cc::Gs {

class IAchievements;
class IFriends;
class IRemoteStorage;
class IStats;
class IUtils;
class IAccount;

enum class GsServicesType : uint8_t {
    Null = 0,
    Steam = 1,
    Epic = 2,
    GsServicesType_Max
};

// Preserve existing public enum values.
enum class ServicesState : uint8_t { Created = 0, Ready, Closing, Closed, Initializing };
enum class ServicesModule : uint8_t { Achievements = 0, Friends, RemoteStorage, Stats, Utils, Account };
#ifdef SWIG
using OnRestartRequired = AsyncCallbackBase;
#else
using OnRestartRequired = AsyncCallback<bool>;
#endif

class IGsServices : public RefCounted {
public:
    virtual ~IGsServices() = default;

    virtual void init(OnComplete callback) = 0;
    virtual ServicesState getState() const = 0;
    virtual bool hasModule(ServicesModule module) const = 0;
    virtual void destroy() = 0;
    virtual void tick(float deltaTime) = 0;
    virtual bool isClosed() const = 0;
    virtual bool isClosing() const = 0;

    virtual GsServicesType getServicesProvider() const = 0;

    virtual cc::IntrusivePtr<IAchievements> getAchievementsInterface() = 0;
    virtual cc::IntrusivePtr<IFriends> getFriendsInterface() = 0;
    virtual cc::IntrusivePtr<IRemoteStorage> getRemoteStorageInterface() = 0;
    virtual cc::IntrusivePtr<IStats> getStatsInterface() = 0;
    virtual cc::IntrusivePtr<IUtils> getUtilsInterface() = 0;
    virtual cc::IntrusivePtr<IAccount> getAccountInterface() = 0;

    virtual void restartAppIfNecessary(const AppId& appId, OnRestartRequired callback) = 0;

    static cc::IntrusivePtr<IGsServices> getServices(GsServicesType servicesType);
};

} // namespace cc::Gs

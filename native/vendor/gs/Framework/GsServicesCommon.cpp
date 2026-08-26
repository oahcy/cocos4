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

#include "commons/GsServicesCommon.h"
#include "commons/GsServicesRegistry.h"
#include "Achievements.h"
#include "Friends.h"
#include "RemoteStorage.h"
#include "Stats.h"
#include "Utils.h"

#include "base/Ptr.h"

namespace cc::Gs {

GsServicesCommon::~GsServicesCommon() = default;

cc::IntrusivePtr<IGsServices> IGsServices::getServices(
    GsServicesType servicesType,
    const std::string& instanceName,
    const std::string& instanceConfigName)
{
    GsServicesRegistry::get().ensureInitialized();
    return GsServicesRegistry::get().getNamedServicesInstance(
        servicesType, instanceName, instanceConfigName);
}

IntrusivePtr<IAchievements> GsServicesCommon::getAchievementsInterface() {
    return get<IAchievements>();
}

IntrusivePtr<IFriends> GsServicesCommon::getFriendsInterface() {
    return get<IFriends>();
}

IntrusivePtr<IRemoteStorage> GsServicesCommon::getRemoteStorageInterface() {
    return get<IRemoteStorage>();
}

IntrusivePtr<IStats> GsServicesCommon::getStatsInterface() {
    return get<IStats>();
}

IntrusivePtr<IUtils> GsServicesCommon::getUtilsInterface() {
    return get<IUtils>();
}

} // namespace cc::Gs

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

namespace cc::Gs {

IntrusivePtr<IGsServices> IGsServices::getServices(GsServicesType provider) {
    return GsServicesRegistry::get().getServicesInstance(provider);
}

GsServicesCommon::GsServicesCommon(GsServicesType provider, std::unique_ptr<GsPlatform> platform)
    : _provider(provider), _session(new GsSession(std::move(platform))) {}
GsServicesCommon::~GsServicesCommon() { _session->close(); }
void GsServicesCommon::init(OnComplete callback) { _session->init(std::move(callback)); }
ServicesState GsServicesCommon::getState() const { return _session->getState(); }
void GsServicesCommon::destroy() { _session->close(); }
void GsServicesCommon::tick(float dt) { _session->tick(dt); }
bool GsServicesCommon::isClosed() const { return _session->isClosed(); }
bool GsServicesCommon::isClosing() const { return _session->isClosing(); }
void GsServicesCommon::restartAppIfNecessary(const AppId& appId, OnRestartRequired callback) { _session->restartAppIfNecessary(appId, std::move(callback)); }

bool GsServicesCommon::hasModule(ServicesModule module) const {
    switch (module) {
    case ServicesModule::Achievements: return _session->achievements() != nullptr;
    case ServicesModule::Friends: return _session->friends() != nullptr;
    case ServicesModule::RemoteStorage: return _session->remoteStorage() != nullptr;
    case ServicesModule::Stats: return _session->stats() != nullptr;
    case ServicesModule::Utils: return _session->utils() != nullptr;
    }
    return false;
}

IntrusivePtr<IAchievements> GsServicesCommon::getAchievementsInterface() {
    if (!_session->achievements()) return nullptr;
    if (!_achievements) _achievements = new IAchievements(_session);
    return _achievements;
}

IntrusivePtr<IFriends> GsServicesCommon::getFriendsInterface() {
    if (!_session->friends()) return nullptr;
    if (!_friends) _friends = new IFriends(_session);
    return _friends;
}

IntrusivePtr<IRemoteStorage> GsServicesCommon::getRemoteStorageInterface() {
    if (!_session->remoteStorage()) return nullptr;
    if (!_remoteStorage) _remoteStorage = new IRemoteStorage(_session);
    return _remoteStorage;
}

IntrusivePtr<IStats> GsServicesCommon::getStatsInterface() {
    if (!_session->stats()) return nullptr;
    if (!_stats) _stats = new IStats(_session);
    return _stats;
}

IntrusivePtr<IUtils> GsServicesCommon::getUtilsInterface() {
    if (!_session->utils()) return nullptr;
    if (!_utils) _utils = new IUtils(_session);
    return _utils;
}

} // namespace cc::Gs

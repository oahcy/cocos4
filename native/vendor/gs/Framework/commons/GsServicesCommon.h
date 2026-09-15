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

#include "GsSession.h"

namespace cc::Gs {

// Small JSB service facade. Business methods live in the module facades.
class GsServicesCommon final : public IGsServices {
public:
    GsServicesCommon(GsServicesType provider, std::unique_ptr<GsPlatform> platform);
    ~GsServicesCommon() override;
    bool init() override;
    void destroy() override;
    void tick(float dt) override;
    bool isClosed() const override;
    bool isClosing() const override;
    GsServicesType getServicesProvider() const override { return _provider; }
    bool restartAppIfNecessary(const AppId& appId) override;
    IntrusivePtr<IAchievements> getAchievementsInterface() override;
    IntrusivePtr<IFriends> getFriendsInterface() override;
    IntrusivePtr<IRemoteStorage> getRemoteStorageInterface() override;
    IntrusivePtr<IStats> getStatsInterface() override;
    IntrusivePtr<IUtils> getUtilsInterface() override;
private:
    GsServicesType _provider;
    IntrusivePtr<GsSession> _session;
    IntrusivePtr<IAchievements> _achievements;
    IntrusivePtr<IFriends> _friends;
    IntrusivePtr<IRemoteStorage> _remoteStorage;
    IntrusivePtr<IStats> _stats;
    IntrusivePtr<IUtils> _utils;
};

} // namespace cc::Gs

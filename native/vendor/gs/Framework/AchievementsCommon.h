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

#include <unordered_map>

#include "Achievements.h"
#include "commons/GsComponent.h"
#include "commons/GsTypes.h"

namespace cc::Gs {

class AchievementsCommon : public GsComponent<IAchievements> {
public:
    using Super = GsComponent<IAchievements>;

    explicit AchievementsCommon(GsServicesCommon& inServices)
        : GsComponent<IAchievements>(inServices) {}

    void setOnAchievementStateUpdated(OnAchievementStateUpdated callback) override {
        if (isShutdown()) return;
        _onUpdatedCallback = std::move(callback);
    }

    // Release the rooted JS listener on shutdown so it can be garbage-collected
    // even if the component outlives the Steam session.
    void shutdown() override {
        _onUpdatedCallback.reset();
        Super::shutdown();
    }

protected:
    using AchievementStateMap = std::unordered_map<std::string, AchievementState>;

    void notifyAchievementUpdated(const AccountId& accountId, const std::string& achievementId, const AchievementState& state) {
        if (_onUpdatedCallback) {
            _onUpdatedCallback.invoke(achievementId, state.Progress, state.UnlockTimeSec);
        }
    }

private:
    OnAchievementStateUpdated _onUpdatedCallback;
};

} // namespace cc::Gs

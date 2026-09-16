#pragma once
#include "../Achievements.h"
#include "GsBackend.h"

namespace cc::Gs {
class IAchievementsBackend : public GsBackend {
public:
    virtual void queryDefinitions(OnAchievementDefinitions callback) = 0;
    virtual void queryStates(OnAchievementStates callback) = 0;
    virtual void unlock(const std::string& id, OnComplete callback) = 0;
    virtual void clearAchievement(const std::string&, OnComplete callback) {
        callback.failure({GsErrorCode::NotSupported, "Achievement reset is not supported"});
    }
    virtual void setOnUpdated(OnAchievementUpdated callback) = 0;
};
} // namespace cc::Gs

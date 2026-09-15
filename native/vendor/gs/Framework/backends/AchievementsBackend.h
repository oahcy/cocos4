#pragma once

#include "../Achievements.h"
#include "GsBackend.h"

namespace cc::Gs {

class IAchievementsBackend : public GsBackend {
public:
    virtual void queryAchievementDefinitions(OnComplete callback) = 0;
    virtual void queryAchievementStates(OnComplete callback) = 0;
    virtual void unlockAchievements(const std::string& achievementId, OnComplete callback) = 0;
    virtual void clearAchievement(const std::string& achievementId, OnComplete callback) = 0;
    virtual AchievementIdsResult getAchievementIds() = 0;
    virtual AchievementDefinitionResult getAchievementDefinition(const std::string& achievementId) = 0;
    virtual AchievementStateResult getAchievementState(const std::string& achievementId) = 0;
    virtual void setOnAchievementStateUpdated(OnAchievementStateUpdated callback) = 0;
};

} // namespace cc::Gs

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "commons/GsCallback.h"
#include "base/RefCounted.h"

namespace cc::Gs {

struct AchievementDefinition {
    std::string AchievementId;
    std::string DisplayName;
    std::string Description;
};

struct AchievementState {
    std::string AchievementId;
    float Progress = 0.0f;
    uint32_t UnlockTimeSec = 0;
};

struct AchievementIdsResult {
    std::vector<std::string> AchievementIds;
};

struct AchievementDefinitionResult {
    AchievementDefinition Definition;
};

struct AchievementStateResult {
    AchievementState State;
};

class IAchievements : public cc::RefCounted {
public:
    virtual ~IAchievements() = default;

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

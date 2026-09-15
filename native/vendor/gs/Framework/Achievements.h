#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "commons/GsCallback.h"
#include "base/RefCounted.h"
#include "base/Ptr.h"

namespace cc::Gs {

class GsSession;

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
    bool Found = false;
    AchievementDefinition Definition;
};

struct AchievementStateResult {
    bool Found = false;
    AchievementState State;
};

// JSB facade: retains its original session, never the platform implementation.
class IAchievements final : public cc::RefCounted {
public:
    ~IAchievements() override;
    void queryAchievementDefinitions(OnComplete callback);
    void queryAchievementStates(OnComplete callback);
    void unlockAchievements(const std::string& achievementId, OnComplete callback);
    void clearAchievement(const std::string& achievementId, OnComplete callback);
    AchievementIdsResult getAchievementIds();
    AchievementDefinitionResult getAchievementDefinition(const std::string& achievementId);
    AchievementStateResult getAchievementState(const std::string& achievementId);
    void setOnAchievementStateUpdated(OnAchievementStateUpdated callback);
#ifndef SWIG
    explicit IAchievements(cc::IntrusivePtr<GsSession> session);
private:
    cc::IntrusivePtr<GsSession> _session;
#endif
};

} // namespace cc::Gs

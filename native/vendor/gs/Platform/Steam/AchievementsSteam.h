#pragma once

#include <optional>
#include <steam_api.h>
#include "../../Framework/AchievementsCommon.h"

namespace cc::Gs {

class AchievementsSteam : public AchievementsCommon {
public:
    using Super = AchievementsCommon;

    explicit AchievementsSteam(GsServicesCommon& inServices)
        : AchievementsCommon(inServices)
        , _cbUserStatsStored(this, &AchievementsSteam::onUserStatsStored)
        , _cbAchievementStored(this, &AchievementsSteam::onAchievementStored) {}

    void initialize() override;
    void shutdown() override;

    void queryAchievementDefinitions(OnComplete callback) override;
    void queryAchievementStates(OnComplete callback) override;
    void unlockAchievements(const std::string& achievementId, OnComplete callback) override;
    void clearAchievement(const std::string& achievementId, OnComplete callback) override;

    AchievementIdsResult getAchievementIds() override;
    AchievementDefinitionResult getAchievementDefinition(const std::string& achievementId) override;
    AchievementStateResult getAchievementState(const std::string& achievementId) override;

private:
    STEAM_CALLBACK(AchievementsSteam, onUserStatsStored, UserStatsStored_t, _cbUserStatsStored);
    STEAM_CALLBACK(AchievementsSteam, onAchievementStored, UserAchievementStored_t, _cbAchievementStored);

    void updateCachedState(ISteamUserStats* stats, const std::string& achievementId, float progressOverride = -1.0f);

    using DefinitionMap = std::unordered_map<std::string, AchievementDefinition>;
    std::optional<DefinitionMap> _definitions;
    AchievementStateMap _states;
};

} // namespace cc::Gs

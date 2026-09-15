#pragma once

#include <optional>
#include <steam_api.h>
#include <unordered_map>
#include "../../Framework/backends/AchievementsBackend.h"

namespace cc::Gs {

class AchievementsSteam : public IAchievementsBackend {
public:
    AchievementsSteam()
        : _cbUserStatsStored(this, &AchievementsSteam::onUserStatsStored)
        , _cbAchievementStored(this, &AchievementsSteam::onAchievementStored) {}

    void setOnAchievementStateUpdated(OnAchievementStateUpdated callback) override { _onUpdatedCallback = std::move(callback); }
    void shutdown() override;
    void invalidateStates() { _states.clear(); }

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

    bool updateCachedState(ISteamUserStats* stats, const std::string& achievementId, float progressOverride = -1.0f);

    using DefinitionMap = std::unordered_map<std::string, AchievementDefinition>;
    std::optional<DefinitionMap> _definitions;
    std::unordered_map<std::string, AchievementState> _states;
    OnAchievementStateUpdated _onUpdatedCallback;
};

} // namespace cc::Gs

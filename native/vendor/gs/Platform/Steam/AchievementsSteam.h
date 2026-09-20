#pragma once
#include <steam_api.h>
#include "../../Framework/backends/AchievementsBackend.h"

namespace cc::Gs {
class AchievementsSteam : public IAchievementsBackend {
public:
    AchievementsSteam()
        : _cbAchievementStored(this, &AchievementsSteam::onAchievementStored) {}
    void shutdown() override;
    void queryDefinitions(OnAchievementDefinitions callback) override;
    void queryStates(OnAchievementStates callback) override;
    void unlock(const std::string& id, OnComplete callback) override;
    void clearAchievement(const std::string& id, OnComplete callback) override;
    void setOnUpdated(OnAchievementUpdated callback) override { _onUpdated = std::move(callback); }
private:
    static bool readState(ISteamUserStats* stats, const std::string& id, AchievementState& state);
    void changeAchievement(const std::string& id, bool unlock, OnComplete callback);
    STEAM_CALLBACK(AchievementsSteam, onAchievementStored, UserAchievementStored_t, _cbAchievementStored);
    OnAchievementUpdated _onUpdated;
};
} // namespace cc::Gs

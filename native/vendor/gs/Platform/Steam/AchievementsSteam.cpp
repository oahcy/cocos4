#include "AchievementsSteam.h"

#include "base/Log.h"

namespace cc::Gs {

void AchievementsSteam::initialize() {
    CC_LOG_INFO("[AchievementsSteam] Initialize (real SDK)");
}

void AchievementsSteam::shutdown() {
    // Unregister Steam callbacks while the session is still alive; their
    // destructors may run after SteamAPI_Shutdown and unregistering here turns
    // those into safe no-ops.
    _cbUserStatsStored.Unregister();
    _cbAchievementStored.Unregister();
    _definitions.reset();
    _states.clear();
    Super::shutdown();
}

void AchievementsSteam::queryAchievementDefinitions(OnComplete callback) {
    if (isShutdown()) {
        callback.failure("Services shut down");
        return;
    }
    auto* stats = SteamUserStats();
    if (!stats) {
        callback.failure("SteamUserStats interface unavailable");
        return;
    }

    uint32 numAch = stats->GetNumAchievements();
    CC_LOG_INFO("[Steam] Found %d achievements", numAch);

    DefinitionMap defs;
    for (uint32 i = 0; i < numAch; ++i) {
        const char* apiName = stats->GetAchievementName(i);
        if (!apiName) continue;

        AchievementDefinition def;
        def.AchievementId = apiName;

        const char* name = stats->GetAchievementDisplayAttribute(apiName, "name");
        def.DisplayName = name ? name : apiName;

        const char* desc = stats->GetAchievementDisplayAttribute(apiName, "desc");
        def.Description = desc ? desc : "";

        defs[apiName] = std::move(def);
    }

    _definitions = std::move(defs);
    callback.success();
}

void AchievementsSteam::queryAchievementStates(OnComplete callback) {
    if (isShutdown()) {
        callback.failure("Services shut down");
        return;
    }
    auto* stats = SteamUserStats();
    if (!stats) {
        callback.failure("SteamUserStats interface unavailable");
        return;
    }

    if (!_definitions.has_value()) {
        callback.failure("Call queryAchievementDefinitions first");
        return;
    }

    for (const auto& pair : *_definitions) {
        updateCachedState(stats, pair.first);
    }

    CC_LOG_INFO("[Steam] Cached %d achievement states", static_cast<int>(_states.size()));
    callback.success();
}

void AchievementsSteam::unlockAchievements(const std::string& achievementId, OnComplete callback) {
    if (isShutdown()) {
        callback.failure("Services shut down");
        return;
    }
    auto* stats = SteamUserStats();
    if (!stats) {
        callback.failure("SteamUserStats interface unavailable");
        return;
    }

    if (!stats->SetAchievement(achievementId.c_str())) {
        callback.failure("SetAchievement failed");
        return;
    }

    CC_LOG_INFO("[Steam] SetAchievement(%s) OK", achievementId.c_str());

    if (!stats->StoreStats()) {
        callback.failure("StoreStats() call failed");
        return;
    }

    updateCachedState(stats, achievementId);
    callback.success();
}

void AchievementsSteam::clearAchievement(const std::string& achievementId, OnComplete callback) {
    if (isShutdown()) {
        callback.failure("Services shut down");
        return;
    }
    auto* stats = SteamUserStats();
    if (!stats) {
        callback.failure("SteamUserStats interface unavailable");
        return;
    }

    if (!stats->ClearAchievement(achievementId.c_str())) {
        callback.failure("ClearAchievement failed");
        return;
    }

    CC_LOG_INFO("[Steam] ClearAchievement(%s) OK", achievementId.c_str());

    if (!stats->StoreStats()) {
        callback.failure("StoreStats() call failed");
        return;
    }

    updateCachedState(stats, achievementId);
    callback.success();
}

void AchievementsSteam::updateCachedState(ISteamUserStats* stats, const std::string& achievementId, float progressOverride) {
    bool achieved = false;
    uint32 unlockTime = 0;
    stats->GetAchievementAndUnlockTime(achievementId.c_str(), &achieved, &unlockTime);

    AchievementState state;
    state.AchievementId = achievementId;
    state.Progress = progressOverride >= 0.0f ? progressOverride : (achieved ? 100.0f : 0.0f);
    state.UnlockTimeSec = unlockTime;
    _states[achievementId] = state;
}

void AchievementsSteam::onUserStatsStored(UserStatsStored_t* pCallback) {
    if (pCallback->m_eResult != k_EResultOK) {
        std::cerr << "    [Steam] StoreStats failed, EResult=" << pCallback->m_eResult << "\n";
        return;
    }
    CC_LOG_INFO("[Steam] StoreStats OK");
}

void AchievementsSteam::onAchievementStored(UserAchievementStored_t* pCallback) {
    const char* achName = pCallback->m_rgchAchievementName;
    uint32 cur = pCallback->m_nCurProgress;
    uint32 max = pCallback->m_nMaxProgress;
    bool isProgress = (cur != 0 || max != 0);

    if (isProgress) {
        CC_LOG_INFO("[Steam] Achievement progress: %s %u/%u", achName, cur, max);
    } else {
        CC_LOG_INFO("[Steam] Achievement fully unlocked: %s", achName);
    }

    auto* stats = SteamUserStats();
    if (!stats) return;

    if (isProgress && max > 0) {
        float pct = static_cast<float>(cur) / static_cast<float>(max) * 100.0f;
        updateCachedState(stats, achName, pct);
    } else {
        updateCachedState(stats, achName);
    }

    auto* user = SteamUser();
    if (!user) return;

    notifyAchievementUpdated(std::to_string(user->GetSteamID().ConvertToUint64()), achName, _states[achName]);
}

AchievementIdsResult
AchievementsSteam::getAchievementIds() {
    if (isShutdown()) return AchievementIdsResult{};
    AchievementIdsResult out;
    if (!_definitions.has_value()) return out;
    for (const auto& pair : *_definitions) {
        out.AchievementIds.push_back(pair.first);
    }
    return out;
}

AchievementDefinitionResult
AchievementsSteam::getAchievementDefinition(const std::string& achievementId) {
    if (isShutdown()) return AchievementDefinitionResult{};
    AchievementDefinitionResult out;
    if (!_definitions.has_value()) return out;
    auto it = _definitions->find(achievementId);
    if (it == _definitions->end()) return out;
    out.Definition = it->second;
    return out;
}

AchievementStateResult
AchievementsSteam::getAchievementState(const std::string& achievementId) {
    if (isShutdown()) return AchievementStateResult{};
    AchievementStateResult out;
    auto it = _states.find(achievementId);
    if (it == _states.end()) return out;
    out.State = it->second;
    return out;
}

} // namespace cc::Gs

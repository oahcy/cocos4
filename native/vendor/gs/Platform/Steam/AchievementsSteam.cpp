#include "AchievementsSteam.h"
#include "base/Log.h"

namespace cc::Gs {
void AchievementsSteam::shutdown() {
    _cbUserStatsStored.Unregister();
    _cbAchievementStored.Unregister();
    _onUpdated.reset();
}

void AchievementsSteam::queryDefinitions(OnAchievementDefinitions callback) {
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "SteamUserStats unavailable"}); return; }
    std::vector<AchievementDefinition> definitions;
    const auto count = stats->GetNumAchievements();
    definitions.reserve(count);
    for (uint32 i = 0; i < count; ++i) {
        const char* id = stats->GetAchievementName(i);
        if (!id || !*id) { callback.failure({GsErrorCode::PlatformError, "Failed to read achievement definition"}); return; }
        const char* name = stats->GetAchievementDisplayAttribute(id, "name");
        const char* description = stats->GetAchievementDisplayAttribute(id, "desc");
        definitions.push_back({id, name ? name : id, description ? description : ""});
    }
    callback.success(std::move(definitions));
}

bool AchievementsSteam::readState(ISteamUserStats* stats, const std::string& id, AchievementState& state) {
    bool unlocked = false;
    uint32 time = 0;
    if (!stats || !stats->GetAchievementAndUnlockTime(id.c_str(), &unlocked, &time)) return false;
    state.id = id;
    state.unlocked = unlocked;
    // Steam's unlock flag does not expose a general achievement progress value.
    state.progress = unlocked ? std::optional<float>(100.0f) : std::nullopt;
    state.unlockedAt = unlocked && time != 0 ? std::optional<int64_t>(time) : std::nullopt;
    return true;
}

void AchievementsSteam::queryStates(OnAchievementStates callback) {
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "SteamUserStats unavailable"}); return; }
    std::vector<AchievementState> states;
    const auto count = stats->GetNumAchievements();
    states.reserve(count);
    for (uint32 i = 0; i < count; ++i) {
        const char* id = stats->GetAchievementName(i);
        AchievementState state;
        if (!id || !*id || !readState(stats, id, state)) {
            callback.failure({GsErrorCode::PlatformError, "Failed to read achievement state"});
            return;
        }
        states.push_back(std::move(state));
    }
    callback.success(std::move(states));
}

void AchievementsSteam::changeAchievement(const std::string& id, bool unlock, OnComplete callback) {
    if (id.empty() || id.find('\0') != std::string::npos) {
        callback.failure({GsErrorCode::InvalidArgument, "Achievement ID must be nonempty and contain no NUL"});
        return;
    }
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "SteamUserStats unavailable"}); return; }
    bool found = false;
    const auto count = stats->GetNumAchievements();
    for (uint32 i = 0; i < count; ++i) {
        const char* name = stats->GetAchievementName(i);
        if (name && id == name) { found = true; break; }
    }
    if (!found) { callback.failure({GsErrorCode::NotFound, "Unknown achievement: " + id}); return; }
    if (!(unlock ? stats->SetAchievement(id.c_str()) : stats->ClearAchievement(id.c_str()))) {
        callback.failure({GsErrorCode::PlatformError, "Failed to modify achievement: " + id});
        return;
    }
    if (!stats->StoreStats()) {
        callback.failure({GsErrorCode::PlatformError, "StoreStats rejected; local achievement state may already have changed"});
        return;
    }
    // Success is SDK acceptance, not server acknowledgement.
    callback.success();
}

void AchievementsSteam::unlock(const std::string& id, OnComplete callback) {
    changeAchievement(id, true, std::move(callback));
}
void AchievementsSteam::clearAchievement(const std::string& id, OnComplete callback) {
    changeAchievement(id, false, std::move(callback));
}
void AchievementsSteam::onUserStatsStored(UserStatsStored_t* result) {
    auto* utils = SteamUtils();
    if (!utils || result->m_nGameID != utils->GetAppID()) return;
    if (result->m_eResult != k_EResultOK) {
        CC_LOG_ERROR("[Steam] StoreStats failed after submission, EResult=%d", static_cast<int>(result->m_eResult));
    }
}
void AchievementsSteam::onAchievementStored(UserAchievementStored_t* result) {
    auto* utils = SteamUtils();
    if (!utils || result->m_nGameID != utils->GetAppID()) return;
    AchievementState state;
    if (!readState(SteamUserStats(), result->m_rgchAchievementName, state)) return;
    if (!state.unlocked && result->m_nMaxProgress > 0) {
        const auto current = result->m_nCurProgress > result->m_nMaxProgress ? result->m_nMaxProgress : result->m_nCurProgress;
        state.progress = static_cast<float>(current) / result->m_nMaxProgress * 100.0f;
    }
    _onUpdated.invoke(state);
}
} // namespace cc::Gs

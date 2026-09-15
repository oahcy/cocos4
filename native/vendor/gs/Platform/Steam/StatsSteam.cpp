#include "StatsSteam.h"

#include "base/Log.h"

namespace cc::Gs {

void StatsSteam::setStatInt(const std::string& name, int32_t value, OnComplete callback) {
    auto* stats = SteamUserStats();
    if (!stats) {
        callback.failure("SteamUserStats interface unavailable");
        return;
    }
    if (!stats->SetStat(name.c_str(), static_cast<int32>(value))) {
        callback.failure("SetStat(int) failed for: " + name);
        return;
    }
    callback.success();
}

void StatsSteam::setStatFloat(const std::string& name, float value, OnComplete callback) {
    auto* stats = SteamUserStats();
    if (!stats) {
        callback.failure("SteamUserStats interface unavailable");
        return;
    }
    if (!stats->SetStat(name.c_str(), value)) {
        callback.failure("SetStat(float) failed for: " + name);
        return;
    }
    callback.success();
}

StatIntResult StatsSteam::getStatInt(const std::string& name) {
    StatIntResult result;
    auto* stats = SteamUserStats();
    if (!stats) return result;
    int32 val = 0;
    if (stats->GetStat(name.c_str(), &val)) {
        result.Success = true;
        result.Value = static_cast<int32_t>(val);
    }
    return result;
}

StatFloatResult StatsSteam::getStatFloat(const std::string& name) {
    StatFloatResult result;
    auto* stats = SteamUserStats();
    if (!stats) return result;
    float val = 0.0f;
    if (stats->GetStat(name.c_str(), &val)) {
        result.Success = true;
        result.Value = val;
    }
    return result;
}

void StatsSteam::storeStats(OnComplete callback) {
    auto* stats = SteamUserStats();
    if (!stats) {
        callback.failure("SteamUserStats interface unavailable");
        return;
    }
    if (!stats->StoreStats()) {
        callback.failure("StoreStats() call failed");
        return;
    }
    callback.success();
}

bool StatsSteam::resetAllStats(bool achievementsToo) {
    auto* stats = SteamUserStats();
    if (!stats) return false;
    bool ok = stats->ResetAllStats(achievementsToo);
    if (ok) {
        if (achievementsToo && _invalidateAchievements) _invalidateAchievements();
        CC_LOG_INFO("[Steam] ResetAllStats(achievementsToo=%d) OK", achievementsToo);
    }
    return ok;
}

} // namespace cc::Gs

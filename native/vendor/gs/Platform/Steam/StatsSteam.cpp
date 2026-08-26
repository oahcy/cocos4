#include "StatsSteam.h"

#include "base/Log.h"

namespace cc::Gs {

void StatsSteam::shutdown() {
    // No Steam callbacks or JS listeners to release; the base flag is what makes
    // post-shutdown calls fail fast.
    Super::shutdown();
}

void StatsSteam::setStatInt(const std::string& name, int32_t value, OnComplete callback) {
    if (isShutdown()) {
        callback.failure("Services shut down");
        return;
    }
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
    if (isShutdown()) {
        callback.failure("Services shut down");
        return;
    }
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
    if (isShutdown()) return {};
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
    if (isShutdown()) return {};
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
    if (isShutdown()) {
        callback.failure("Services shut down");
        return;
    }
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
    if (isShutdown()) return false;
    auto* stats = SteamUserStats();
    if (!stats) return false;
    bool ok = stats->ResetAllStats(achievementsToo);
    if (ok) {
        CC_LOG_INFO("[Steam] ResetAllStats(achievementsToo=%d) OK", achievementsToo);
    }
    return ok;
}

} // namespace cc::Gs

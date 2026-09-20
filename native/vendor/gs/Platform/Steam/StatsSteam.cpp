#include "StatsSteam.h"
#include "base/Log.h"

#include <cmath>
#include <limits>

namespace cc::Gs {
namespace {
bool validName(const std::string& name) {
    return !name.empty() && name.find('\0') == std::string::npos;
}
bool validFloat(double value) {
    return std::isfinite(value) && std::abs(value) <= std::numeric_limits<float>::max();
}
constexpr int64_t MIN_INT = std::numeric_limits<int32_t>::min();
constexpr int64_t MAX_INT = std::numeric_limits<int32_t>::max();
} // namespace

void StatsSteam::shutdown() {
    _cbUserStatsStored.Unregister();
    _diagnostic = nullptr;
}

void StatsSteam::onUserStatsStored(UserStatsStored_t* result) {
    auto* utils = SteamUtils();
    if (!result || !utils || result->m_nGameID != utils->GetAppID() || result->m_eResult == k_EResultOK) return;
    std::string message = "StoreStats failed after submission, EResult=" + std::to_string(static_cast<int>(result->m_eResult));
    if (result->m_eResult == k_EResultInvalidParam) {
        message += "; query stats again to observe server-corrected values; no automatic retry";
    }
    CC_LOG_ERROR("[Steam] %s", message.c_str());
    if (_diagnostic) _diagnostic({DiagnosticLevel::Error, std::move(message)});
}

void StatsSteam::getInt(const std::string& name, OnStatInt callback) {
    if (!validName(name)) { callback.failure({GsErrorCode::InvalidArgument, "Invalid stat name"}); return; }
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "Stats unavailable"}); return; }
    int32 value = 0;
    if (!stats->GetStat(name.c_str(), &value)) {
        // Steam does not distinguish a missing name, wrong type, or unavailable data.
        callback.failure({GsErrorCode::PlatformError, "Integer stat lookup failed: " + name}); return;
    }
    callback.success(value);
}

void StatsSteam::getFloat(const std::string& name, OnStatFloat callback) {
    if (!validName(name)) { callback.failure({GsErrorCode::InvalidArgument, "Invalid stat name"}); return; }
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "Stats unavailable"}); return; }
    float value = 0;
    if (!stats->GetStat(name.c_str(), &value) || !std::isfinite(value)) {
        callback.failure({GsErrorCode::PlatformError, "Float stat lookup failed: " + name}); return;
    }
    callback.success(value);
}

void StatsSteam::setInt(const std::string& name, int64_t value, OnComplete callback) {
    if (!validName(name) || value < MIN_INT || value > MAX_INT) {
        callback.failure({GsErrorCode::InvalidArgument, "Expected a stat name and signed 32-bit integer"}); return;
    }
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "Stats unavailable"}); return; }
    if (!stats->SetStat(name.c_str(), static_cast<int32>(value))) {
        callback.failure({GsErrorCode::PlatformError, "Integer stat update rejected: " + name}); return;
    }
    callback.success();
}

void StatsSteam::setFloat(const std::string& name, double value, OnComplete callback) {
    if (!validName(name) || !validFloat(value)) {
        callback.failure({GsErrorCode::InvalidArgument, "Expected a stat name and finite float32 value"}); return;
    }
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "Stats unavailable"}); return; }
    if (!stats->SetStat(name.c_str(), static_cast<float>(value))) {
        callback.failure({GsErrorCode::PlatformError, "Float stat update rejected: " + name}); return;
    }
    callback.success();
}

void StatsSteam::incrementInt(const std::string& name, int64_t delta, OnComplete callback) {
    if (!validName(name)) { callback.failure({GsErrorCode::InvalidArgument, "Invalid stat name"}); return; }
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "Stats unavailable"}); return; }
    int32 current = 0;
    if (!stats->GetStat(name.c_str(), &current)) {
        callback.failure({GsErrorCode::PlatformError, "Integer stat lookup failed: " + name}); return;
    }
    // Check before addition, including calls made directly from native code.
    if (delta < MIN_INT - current || delta > MAX_INT - current) {
        callback.failure({GsErrorCode::InvalidArgument, "Integer stat increment would overflow"}); return;
    }
    // Read and write stay in this engine-thread dispatch. Not a cross-device atomic increment.
    setInt(name, static_cast<int64_t>(current) + delta, std::move(callback));
}

void StatsSteam::incrementFloat(const std::string& name, double delta, OnComplete callback) {
    if (!validName(name) || !std::isfinite(delta)) {
        callback.failure({GsErrorCode::InvalidArgument, "Expected a stat name and finite delta"}); return;
    }
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "Stats unavailable"}); return; }
    float current = 0;
    if (!stats->GetStat(name.c_str(), &current) || !std::isfinite(current)) {
        callback.failure({GsErrorCode::PlatformError, "Float stat lookup failed: " + name}); return;
    }
    setFloat(name, static_cast<double>(current) + delta, std::move(callback));
}

void StatsSteam::flush(OnComplete callback) {
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "Stats unavailable"}); return; }
    if (!stats->StoreStats()) {
        callback.failure({GsErrorCode::PlatformError, "Stat submission rejected"}); return;
    }
    callback.success(); // SDK accepted submission, not server acknowledgement.
}

void StatsSteam::resetAll(bool includeAchievements, OnComplete callback) {
    auto* stats = SteamUserStats();
    if (!stats) { callback.failure({GsErrorCode::NotReady, "Stats unavailable"}); return; }
    if (!stats->ResetAllStats(includeAchievements)) {
        callback.failure({GsErrorCode::PlatformError, "Stat reset rejected"}); return;
    }
    callback.success(); // Platform accepted reset; no rollback or server acknowledgement.
}

} // namespace cc::Gs

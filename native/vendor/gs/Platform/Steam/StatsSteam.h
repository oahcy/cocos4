#pragma once
#include <steam_api.h>
#include <functional>
#include "../../Framework/Utils.h"
#include "../../Framework/backends/StatsBackend.h"

namespace cc::Gs {
class StatsSteam : public IStatsBackend {
public:
    using DiagnosticSink = std::function<void(DiagnosticMessage)>;
    explicit StatsSteam(DiagnosticSink diagnostic)
        : _cbUserStatsStored(this, &StatsSteam::onUserStatsStored), _diagnostic(std::move(diagnostic)) {}
    void shutdown() override;
    void getInt(const std::string& name, OnStatInt callback) override;
    void getFloat(const std::string& name, OnStatFloat callback) override;
    void setInt(const std::string& name, int64_t value, OnComplete callback) override;
    void setFloat(const std::string& name, double value, OnComplete callback) override;
    void incrementInt(const std::string& name, int64_t delta, OnComplete callback) override;
    void incrementFloat(const std::string& name, double delta, OnComplete callback) override;
    void flush(OnComplete callback) override;
    void resetAll(bool includeAchievements, OnComplete callback) override;
private:
    STEAM_CALLBACK(StatsSteam, onUserStatsStored, UserStatsStored_t, _cbUserStatsStored);
    DiagnosticSink _diagnostic;
};
} // namespace cc::Gs

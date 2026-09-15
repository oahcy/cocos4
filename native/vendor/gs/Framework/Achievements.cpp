#include "Achievements.h"
#include "commons/GsSession.h"

namespace cc::Gs {

IAchievements::IAchievements(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IAchievements::~IAchievements() = default;

void IAchievements::queryAchievementDefinitions(OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->achievements();
    if (!backend) {
        callback.failure("Achievements unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->queryAchievementDefinitions(std::move(callback));
}

void IAchievements::queryAchievementStates(OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->achievements();
    if (!backend) {
        callback.failure("Achievements unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->queryAchievementStates(std::move(callback));
}

void IAchievements::unlockAchievements(const std::string& achievementId, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->achievements();
    if (!backend) {
        callback.failure("Achievements unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->unlockAchievements(achievementId, std::move(callback));
}

void IAchievements::clearAchievement(const std::string& achievementId, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->achievements();
    if (!backend) {
        callback.failure("Achievements unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->clearAchievement(achievementId, std::move(callback));
}

AchievementIdsResult IAchievements::getAchievementIds() {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->achievements();
    if (!backend) {
        return {};
    }
    return backend->getAchievementIds();
}

AchievementDefinitionResult IAchievements::getAchievementDefinition(const std::string& achievementId) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->achievements();
    if (!backend) {
        return {};
    }
    return backend->getAchievementDefinition(achievementId);
}

AchievementStateResult IAchievements::getAchievementState(const std::string& achievementId) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->achievements();
    if (!backend) {
        return {};
    }
    return backend->getAchievementState(achievementId);
}

void IAchievements::setOnAchievementStateUpdated(OnAchievementStateUpdated callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->achievements();
    if (!backend) {
        return;
    }
    callback.setGate(_session->gate());
    backend->setOnAchievementStateUpdated(std::move(callback));
}

} // namespace cc::Gs

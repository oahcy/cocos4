#include "Achievements.h"
#include "commons/GsSession.h"

namespace cc::Gs {

IAchievements::IAchievements(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IAchievements::~IAchievements() = default;

void IAchievements::queryDefinitions(OnAchievementDefinitions callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->achievements(), callback, "Achievements");
    if (!backend) return;
    backend->queryDefinitions(std::move(callback));
}

void IAchievements::queryStates(OnAchievementStates callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->achievements(), callback, "Achievements");
    if (!backend) return;
    backend->queryStates(std::move(callback));
}

void IAchievements::unlock(const std::string& id, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->achievements(), callback, "Achievements");
    if (!backend) return;
    backend->unlock(id, std::move(callback));
}

void IAchievements::clearAchievement(const std::string& id, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->achievements(), callback, "Achievements");
    if (!backend) return;
    backend->clearAchievement(id, std::move(callback));
}

void IAchievements::setOnUpdated(OnAchievementUpdated callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->achievements();
    if (!backend) return;
    callback.setGate(_session->gate());
    backend->setOnUpdated(std::move(callback));
}
} // namespace cc::Gs

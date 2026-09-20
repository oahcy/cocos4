#include "Stats.h"
#include "commons/GsSession.h"

namespace cc::Gs {
IStats::IStats(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IStats::~IStats() = default;

void IStats::getInt(const std::string& name, OnStatInt callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->stats(), callback, "Stats");
    if (!backend) return;
    backend->getInt(name, std::move(callback));
}

void IStats::getFloat(const std::string& name, OnStatFloat callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->stats(), callback, "Stats");
    if (!backend) return;
    backend->getFloat(name, std::move(callback));
}

void IStats::setInt(const std::string& name, int64_t value, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->stats(), callback, "Stats");
    if (!backend) return;
    backend->setInt(name, value, std::move(callback));
}

void IStats::setFloat(const std::string& name, double value, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->stats(), callback, "Stats");
    if (!backend) return;
    backend->setFloat(name, value, std::move(callback));
}

void IStats::incrementInt(const std::string& name, int64_t delta, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->stats(), callback, "Stats");
    if (!backend) return;
    backend->incrementInt(name, delta, std::move(callback));
}

void IStats::incrementFloat(const std::string& name, double delta, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->stats(), callback, "Stats");
    if (!backend) return;
    backend->incrementFloat(name, delta, std::move(callback));
}

void IStats::flush(OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->stats(), callback, "Stats");
    if (!backend) return;
    backend->flush(std::move(callback));
}

void IStats::resetAll(bool includeAchievements, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->stats(), callback, "Stats");
    if (!backend) return;
    backend->resetAll(includeAchievements, std::move(callback));
}
} // namespace cc::Gs

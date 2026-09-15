#include "Stats.h"
#include "commons/GsSession.h"

namespace cc::Gs {

IStats::IStats(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IStats::~IStats() = default;

void IStats::setStatInt(const std::string& name, int32_t value, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        callback.failure("Stats unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->setStatInt(name, value, std::move(callback));
}

void IStats::setStatFloat(const std::string& name, float value, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        callback.failure("Stats unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->setStatFloat(name, value, std::move(callback));
}

StatIntResult IStats::getStatInt(const std::string& name) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        return {};
    }
    return backend->getStatInt(name);
}

StatFloatResult IStats::getStatFloat(const std::string& name) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        return {};
    }
    return backend->getStatFloat(name);
}

void IStats::storeStats(OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        callback.failure("Stats unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->storeStats(std::move(callback));
}

bool IStats::resetAllStats(bool achievementsToo) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        return {};
    }
    return backend->resetAllStats(achievementsToo);
}

} // namespace cc::Gs

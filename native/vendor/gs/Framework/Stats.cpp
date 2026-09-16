#include "Stats.h"
#include "commons/GsSession.h"

namespace cc::Gs {
IStats::IStats(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IStats::~IStats() = default;

void IStats::getInt(const std::string& name, OnStatInt callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Stats unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->getInt(name, std::move(callback));
}

void IStats::getFloat(const std::string& name, OnStatFloat callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Stats unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->getFloat(name, std::move(callback));
}

void IStats::setInt(const std::string& name, int64_t value, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Stats unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->setInt(name, value, std::move(callback));
}

void IStats::setFloat(const std::string& name, double value, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Stats unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->setFloat(name, value, std::move(callback));
}

void IStats::incrementInt(const std::string& name, int64_t delta, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Stats unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->incrementInt(name, delta, std::move(callback));
}

void IStats::incrementFloat(const std::string& name, double delta, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Stats unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->incrementFloat(name, delta, std::move(callback));
}

void IStats::flush(OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Stats unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->flush(std::move(callback));
}

void IStats::resetAll(bool includeAchievements, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->stats();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Stats unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->resetAll(includeAchievements, std::move(callback));
}
} // namespace cc::Gs

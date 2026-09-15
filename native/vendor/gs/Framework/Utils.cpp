#include "Utils.h"
#include "commons/GsSession.h"

namespace cc::Gs {

IUtils::IUtils(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IUtils::~IUtils() = default;

void IUtils::setWarningMessageHook(OnWarningMessage callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->utils();
    if (!backend) {
        return;
    }
    callback.setGate(_session->gate());
    backend->setWarningMessageHook(std::move(callback));
}

} // namespace cc::Gs

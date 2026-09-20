#include "Account.h"
#include "commons/GsSession.h"

namespace cc::Gs {
IAccount::IAccount(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IAccount::~IAccount() = default;

void IAccount::getUser(OnAccountUser callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->account(), callback, "Account");
    if (!backend) return;
    backend->getUser(std::move(callback));
}

void IAccount::login(OnLogin callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->account(), callback, "Account");
    if (!backend) return;
    backend->login(std::move(callback));
}

} // namespace cc::Gs

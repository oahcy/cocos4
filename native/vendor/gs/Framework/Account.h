#pragma once
#include <optional>
#include "commons/GsCallback.h"
#include "commons/GsTypes.h"
#include "base/RefCounted.h"
#include "base/Ptr.h"

namespace cc::Gs {
class GsSession;
#ifdef SWIG
using OnAccountUser = AsyncCallbackBase;
using OnLogin = AsyncCallbackBase;
#else
using OnAccountUser = AsyncCallback<std::optional<UserProfile>>;
using OnLogin = AsyncCallback<UserProfile>;
#endif

// The facade remains bound to its original session after close.
class IAccount final : public cc::RefCounted {
public:
    ~IAccount() override;
    void getUser(OnAccountUser callback);
    void login(OnLogin callback);
#ifndef SWIG
    explicit IAccount(cc::IntrusivePtr<GsSession> session);
private:
    cc::IntrusivePtr<GsSession> _session;
#endif
};
} // namespace cc::Gs

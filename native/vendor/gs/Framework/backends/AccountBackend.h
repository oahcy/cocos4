#pragma once
#include "../Account.h"
#include "GsBackend.h"

namespace cc::Gs {
class IAccountBackend : public GsBackend {
public:
    // Null means no local identity; it does not describe network connectivity.
    virtual void getUser(OnAccountUser callback) = 0;
    // Optional interactive login. Already signed in: return the same user.
    // Only one login attempt at a time; concurrent attempts reject Busy.
    // Switching identity requires a new service session, not mutation of this one.
    virtual void login(OnLogin callback) {
        callback.failure({GsErrorCode::NotSupported, "In-game login is not supported"});
    }
};
} // namespace cc::Gs

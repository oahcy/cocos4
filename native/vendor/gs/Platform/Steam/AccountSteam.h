#pragma once
#include "../../Framework/backends/AccountBackend.h"

namespace cc::Gs {
class AccountSteam final : public IAccountBackend {
public:
    void getUser(OnAccountUser callback) override;
};
} // namespace cc::Gs

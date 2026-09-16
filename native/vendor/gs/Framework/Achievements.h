#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include "commons/GsCallback.h"
#include "base/RefCounted.h"
#include "base/Ptr.h"

namespace cc::Gs {
class GsSession;
struct AchievementDefinition {
    std::string id;
    std::string displayName;
    std::string description;
};
struct AchievementState {
    std::string id;
    bool unlocked = false;
    std::optional<float> progress; // Percent [0, 100], absent if unknown.
    std::optional<int64_t> unlockedAt; // Unix seconds; absent if unknown/locked.
};
#ifdef SWIG
using OnAchievementDefinitions = AsyncCallbackBase;
using OnAchievementStates = AsyncCallbackBase;
using OnAchievementUpdated = EventDelegateBase;
#else
using OnAchievementDefinitions = AsyncCallback<std::vector<AchievementDefinition>>;
using OnAchievementStates = AsyncCallback<std::vector<AchievementState>>;
using OnAchievementUpdated = EventDelegate<AchievementState>;
#endif
// Each facade is permanently bound to its original session.
class IAchievements final : public cc::RefCounted {
public:
    ~IAchievements() override;
    void queryDefinitions(OnAchievementDefinitions callback);
    void queryStates(OnAchievementStates callback);
    void unlock(const std::string& id, OnComplete callback);
    void clearAchievement(const std::string& id, OnComplete callback);
    void setOnUpdated(OnAchievementUpdated callback);
#ifndef SWIG
    explicit IAchievements(cc::IntrusivePtr<GsSession> session);
private:
    cc::IntrusivePtr<GsSession> _session;
#endif
};
} // namespace cc::Gs

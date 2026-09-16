#pragma once

#include <memory>
#include <optional>
#include <vector>
#include "GsServices.h"
#include "../backends/AchievementsBackend.h"
#include "../backends/FriendsBackend.h"
#include "../backends/RemoteStorageBackend.h"
#include "../backends/StatsBackend.h"
#include "../backends/UtilsBackend.h"

namespace cc::Gs {

struct GsModules {
    std::unique_ptr<IAchievementsBackend> achievements;
    std::unique_ptr<IFriendsBackend> friends;
    std::unique_ptr<IRemoteStorageBackend> remoteStorage;
    std::unique_ptr<IStatsBackend> stats;
    std::unique_ptr<IUtilsBackend> utils;
    void shutdown();
};

// SDK-specific preparation and teardown stay outside the generic session.
// On initialization failure the platform must undo any SDK resources it acquired.
class GsPlatform {
public:
    virtual ~GsPlatform() = default;
    virtual std::optional<GsError> initialize(GsModules& modules) = 0;
    virtual void pump(float dt) = 0;
    virtual void shutdown() = 0;
    // Like initialize(), launcher preparation completes within this call.
    virtual void restartAppIfNecessary(const AppId&, OnRestartRequired callback) {
        callback.failure({GsErrorCode::NotSupported, "Launcher restart is not supported"});
    }
};

// All operations run on the engine thread. Module facades retain this object,
// but it never owns facades, so closing releases the backends without a cycle.
class GsSession final : public RefCounted {
public:
    using State = ServicesState;
    explicit GsSession(std::unique_ptr<GsPlatform> platform);
    ~GsSession() override;
    void init(OnComplete callback);
    ServicesState getState() const { return _state; }
    void close();
    void tick(float dt);
    void restartAppIfNecessary(const AppId& appId, OnRestartRequired callback);
    bool isClosed() const { return _state == State::Closed; }
    bool isClosing() const { return _state == State::Closing; }
    bool isActive() const { return _state == State::Ready; }

    class Dispatch {
    public:
        explicit Dispatch(GsSession& session) : _session(&session) { ++session._dispatchDepth; }
        ~Dispatch();
        Dispatch(const Dispatch&) = delete;
        Dispatch& operator=(const Dispatch&) = delete;
    private:
        IntrusivePtr<GsSession> _session;
    };

    IAchievementsBackend* achievements() const { return isActive() ? _modules.achievements.get() : nullptr; }
    IFriendsBackend* friends() const { return isActive() ? _modules.friends.get() : nullptr; }
    IRemoteStorageBackend* remoteStorage() const { return isActive() ? _modules.remoteStorage.get() : nullptr; }
    IStatsBackend* stats() const { return isActive() ? _modules.stats.get() : nullptr; }
    IUtilsBackend* utils() const { return isActive() ? _modules.utils.get() : nullptr; }
    const std::shared_ptr<SessionGate>& gate() const { return _gate; }
    void track(const std::shared_ptr<PendingCallback>& pending);

private:
    void finishClose();
    std::unique_ptr<GsPlatform> _platform;
    GsModules _modules;
    std::shared_ptr<SessionGate> _gate = std::make_shared<SessionGate>();
    std::vector<std::weak_ptr<PendingCallback>> _pending;
    State _state = State::Created;
    unsigned _dispatchDepth = 0;
    bool _sdkInitialized = false;
    bool _finishingClose = false;
};

} // namespace cc::Gs

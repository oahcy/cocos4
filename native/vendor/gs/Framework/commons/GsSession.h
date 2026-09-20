#pragma once

#include <memory>
#include <chrono>
#include <optional>
#include <vector>
#include "GsServices.h"
#include "../backends/AchievementsBackend.h"
#include "../backends/FriendsBackend.h"
#include "../backends/RemoteStorageBackend.h"
#include "../backends/StatsBackend.h"
#include "../backends/UtilsBackend.h"
#include "../backends/AccountBackend.h"

namespace cc::Gs {

struct GsModules {
    std::unique_ptr<IAccountBackend> account;
    std::unique_ptr<IAchievementsBackend> achievements;
    std::unique_ptr<IFriendsBackend> friends;
    std::unique_ptr<IRemoteStorageBackend> remoteStorage;
    std::unique_ptr<IStatsBackend> stats;
    std::unique_ptr<IUtilsBackend> utils;
    void shutdown();
};

// SDK-specific preparation and teardown stay outside the generic session.
// initialize may complete inline or later on the engine thread. pump must work while initializing.
// shutdown must cancel initialization, unregister callbacks, and stop all access to modules.
// It is also called after failed/partial initialization and must be safe in that state.
class GsPlatform {
public:
    virtual ~GsPlatform() = default;
    virtual void initialize(GsModules& modules, OnComplete callback) = 0;
    virtual void pump(float dt) = 0;
    virtual void shutdown() = 0;
    // Launcher preparation still completes within this call.
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
    IAccountBackend* account() const { return isActive() ? _modules.account.get() : nullptr; }
    IUtilsBackend* utils() const { return isActive() ? _modules.utils.get() : nullptr; }
    const std::shared_ptr<SessionGate>& gate() const { return _gate; }
    // No timeout unless supplied. Timeout settles the callback, not the SDK operation.
    void track(const std::shared_ptr<PendingCallback>& pending,
               std::optional<std::chrono::milliseconds> timeout = std::nullopt);

    // Call under Dispatch and keep it alive through the backend invocation.
    template<class Backend, typename... Args>
    Backend* prepare(Backend* backend, const AsyncCallback<Args...>& callback,
                     const char* module, std::optional<std::chrono::milliseconds> timeout = std::nullopt) {
        if (!backend) {
            const auto code = isClosed() || isClosing() ? GsErrorCode::Cancelled
                : (isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
            callback.failure({code, std::string(module) + " unavailable in this session"});
            return nullptr;
        }
        track(callback.pending(), timeout);
        return backend;
    }

private:
    struct Initialization {
        bool completed = false;
        std::optional<GsError> error;
    };
    void finishInitialization();
    void finishClose();
    void expireRequests();
    struct PendingRequest {
        std::weak_ptr<PendingCallback> callback;
        std::optional<std::chrono::steady_clock::time_point> deadline;
    };
    std::unique_ptr<GsPlatform> _platform;
    GsModules _modules;
    std::shared_ptr<SessionGate> _gate = std::make_shared<SessionGate>();
    std::vector<PendingRequest> _pending;
    std::chrono::steady_clock::time_point _nextPendingCheck{};
    State _state = State::Created;
    unsigned _dispatchDepth = 0;
    // Completion callbacks hold only a weak token, never a Session pointer.
    std::shared_ptr<Initialization> _initialization;
    std::vector<OnComplete> _initWaiters;
    bool _platformStarted = false;
    bool _finishingClose = false;
};

} // namespace cc::Gs

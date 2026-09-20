#pragma once

#include <steam_api.h>
#include <mutex>
#include <vector>
#include "../../Framework/backends/UtilsBackend.h"

namespace cc::Gs {

class UtilsSteam : public IUtilsBackend {
public:

    void shutdown() override;
    void update();
    // Internal platform diagnostics share the deferred SDK-warning queue.
    static void enqueueDiagnostic(DiagnosticMessage message);

    void setOnDiagnostic(OnDiagnostic callback) override;

private:
    static void steamWarningHook(int severity, const char* msg);

    static OnDiagnostic s_callback;
    static std::mutex s_mutex;
    static std::vector<DiagnosticMessage> s_messages;
    static bool s_acceptMessages;
    static uint64_t s_generation; // Engine-thread subscription generation.
};

} // namespace cc::Gs

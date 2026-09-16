#pragma once
#include <string>
#include <utility>

namespace cc::Gs {
// Values are part of the JSB contract; keep aligned with core/errors.ts.
enum class GsErrorCode {
    NotSupported = 0, NotReady = 1, NotFound = 2, InvalidArgument = 3,
    Busy = 4, Cancelled = 5, Timeout = 6, PlatformError = 7
};
struct GsError {
    GsErrorCode code = GsErrorCode::PlatformError;
    std::string message;
    std::string platformCode;
    GsError(GsErrorCode code, std::string message, std::string platformCode = {})
        : code(code), message(std::move(message)), platformCode(std::move(platformCode)) {}
};
} // namespace cc::Gs

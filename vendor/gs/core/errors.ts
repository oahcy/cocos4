// Values are shared with native Framework/commons/GsError.h.
export enum GsErrorCode {
    NotSupported = 0,
    NotReady = 1,
    NotFound = 2,
    InvalidArgument = 3,
    Busy = 4,
    Cancelled = 5,
    Timeout = 6,
    PlatformError = 7,
}

export interface NativeGsError {
    code: GsErrorCode;
    message: string;
    platformCode?: string | null;
}

export class GsError extends Error {
    constructor (
        public readonly code: GsErrorCode,
        message: string,
        public readonly provider: number,
        public readonly platformCode: string | null = null,
    ) {
        super(message);
        this.name = 'GsError';
    }
}

/** @internal */
export function toGsError (error: unknown, provider: number): GsError {
    if (error instanceof GsError) return error;
    if (error && typeof error === 'object' && 'code' in error && 'message' in error) {
        const native = error as NativeGsError;
        return new GsError(native.code, native.message, provider, native.platformCode ?? null);
    }
    return new GsError(GsErrorCode.PlatformError, error instanceof Error ? error.message : String(error), provider);
}

/* eslint-disable @typescript-eslint/no-namespace */
/*
 Copyright (c) 2025 Xiamen Yaji Software Co., Ltd.

 https://www.cocos.com/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

import './core/enums';

export declare namespace gs {
    /** Stable error codes shared by service operations. */
    export enum GsErrorCode {
        NotSupported = 0, NotReady = 1, NotFound = 2, InvalidArgument = 3,
        Busy = 4, Cancelled = 5, Timeout = 6, PlatformError = 7,
    }

    export class GsError extends Error {
        readonly code: GsErrorCode;
        readonly provider: ServicesProvider;
        readonly platformCode: string | null;
        constructor(code: GsErrorCode, message: string, provider: ServicesProvider, platformCode?: string | null);
    }


    // ────────────────────────────────────────────────────
    // Enums
    // ────────────────────────────────────────────────────

    /**
     * @en Platform services provider type.
     * @zh 平台服务提供商类型。
     */
    export enum ServicesProvider {
        /**
         * @en No provider; getServices returns null.
         * @zh 未指定提供商，getServices 返回 null。
         */
        Null = 0,
        /**
         * @en Steam (Steamworks SDK).
         * @zh Steam（Steamworks SDK）。
         */
        Steam = 1,
        /**
         * @en Epic Online Services. NOT IMPLEMENTED YET — reserved for future use.
         *     No native module/factory is registered for this type, so
         *     `getServices(ServicesProvider.Epic)` always returns null.
         * @zh Epic 在线服务。**尚未实现**——目前仅预留枚举值。
         *     没有为该类型注册任何 native 模块/工厂，因此
         *     `getServices(ServicesProvider.Epic)` 始终返回 null。
         */
        Epic = 2,
    }

    // ────────────────────────────────────────────────────
    // Data types
    // ────────────────────────────────────────────────────

    /** Provider-independent observed presence; Unknown means unavailable. */
    export enum PresenceState { Unknown = 0, Offline, Online, Away, Busy }
    /** Preferred size; providers may return the closest available resolution. */
    export enum AvatarSize { Small = 0, Medium, Large }
    /** Optional overlay pages. Unsupported pages reject with NotSupported. */
    export enum OverlayDialog { Friends = 0, Community, Players, Settings, OfficialGameGroup, Stats, Achievements }

    /** IDs are opaque strings scoped to the current provider. */
    export interface UserProfile { userId: string; displayName: string; }
    export interface FriendInfo extends UserProfile {
        /** Current user's nickname for this friend; null when absent or unavailable. */
        nickname: string | null;
        presence: PresenceState;
    }
    /** Independent RGBA8 pixels, tightly packed; data.length = width * height * 4. */
    export interface AvatarImage { width: number; height: number; data: Uint8Array; }
    export interface FriendGroup { id: string; displayName: string; memberIds: string[]; }
    export interface JoinRequest {
        userId: string;
        /** Opaque connection data defined by the game. */
        connectionString: string;
    }

    /**
     * @en Achievement definition data.
     * @zh 成就定义数据。
     */
    export interface AchievementDefinition {
        id: string;
        displayName: string;
        description: string;
    }

    /**
     * @en Achievement state data.
     * @zh 成就状态数据。
     */
    export interface AchievementState {
        id: string;
        unlocked: boolean;
        /** Percentage in [0, 100], or null when unknown. */
        progress: number | null;
        /** Unix seconds; null when locked or the time is unknown. */
        unlockedAt: number | null;
    }

    /**
     * @en File info returned by listFiles().
     * @zh listFiles() 返回的文件信息。
     */
    export interface FileInfo {
        name: string;
        size: number;
    }

    /**
     * @en Cloud storage quota info.
     * @zh 云存储配额信息。
     */
    export interface QuotaInfo {
        totalBytes: number;
        availableBytes: number;
    }

    export interface ResetStatsOptions {
        /** Defaults to false. Only set true to reset achievements together with stats. */
        includeAchievements?: boolean;
    }

    // ────────────────────────────────────────────────────
    // Entry point
    // ────────────────────────────────────────────────────

    /**
     * @en Get or create the single game services instance for a provider.
     *     Returns null if no factory is registered for the given type.
     *     Call init() explicitly after restartAppIfNecessary check.
     * @zh 获取或创建指定平台的唯一游戏服务实例。
     *     未注册对应平台工厂时返回 null。
     *     需在 restartAppIfNecessary 检查后显式调用 init()。
     * @param servicesType - The provider type (default: Steam).
     */
    export function getServices (
        servicesType?: ServicesProvider,
    ): Services | null;

    // ────────────────────────────────────────────────────
    // IGsServices
    // ────────────────────────────────────────────────────

    /**
     * @en Game services instance providing lifecycle management and sub-interface access.
     * @zh 游戏服务实例，提供生命周期管理和子接口访问。
     */
    export enum ServicesState { Created = 0, Ready, Closing, Closed }
    export enum ServicesModule { Achievements = 0, Friends, RemoteStorage, Stats, Utils }

    export class Services {
        /** Optional launcher check, only before init(). True means relaunch was requested:
         * exit the current process. False means no relaunch was requested; it does not prove
         * authentication or online connectivity. Failure rejects GsError, unsupported providers
         * reject NotSupported. Steam requires a positive uint32 App ID; other providers may
         * accept a string. Use the app's configured published ID.
         */
        restartAppIfNecessary (appId: number | string): Promise<boolean>;
        /** Initializes the SDK session. Repeated calls when Ready succeed.
         * Failure rejects GsError and leaves Created for retry; a closed session cannot reopen.
         * Ready does not imply online connectivity, user authentication, or synchronized data.
         * This API allows asynchronous completion; Steam currently initializes synchronously.
         */
        init (): Promise<void>;
        /** Native session snapshot; readable after shutdown. */
        getState (): ServicesState;
        /** Available modules in this initialized session. NotReady before init, Cancelled after close.
         * A present module may still reject optional operations with NotSupported.
         */
        hasModule (module: ServicesModule): boolean;
        /** Idempotent close. Cancels pending operations; callback dispatch may defer resource release. */
        destroy (): void;
        /** Provider identity, also readable after shutdown. */
        getServicesProvider (): ServicesProvider;
        /** Module getters throw NotReady before init, NotSupported for an absent module, Cancelled after close. */
        achievements (): Achievements;
        friends (): Friends;
        remoteStorage (): RemoteStorage;
        stats (): Stats;
        utils (): Utils;
    }

    // ────────────────────────────────────────────────────
    // IAchievements
    // ────────────────────────────────────────────────────

    /**
     * @en Provides access to platform achievements (query, unlock, state).
     * @zh 提供平台成就的访问（查询、解锁、状态）。
     */
    export class Achievements {
        /** Returns a fresh snapshot of definitions. No prior query is required. */
        queryDefinitions (): Promise<AchievementDefinition[]>;
        /** Returns a fresh snapshot of current-user states, independently of definitions. */
        queryStates (): Promise<AchievementState[]>;
        /**
         * Requests an unlock. Resolves when the platform accepts the change and submission.
         * Does not guarantee server persistence. A rejected submission may follow a local change.
         * Rejects with GsError; an unknown ID is NotFound.
         */
        unlock (id: string): Promise<void>;
        /**
         * Optional reset operation; rejects with NotSupported if unavailable.
         * Has the same completion boundary as unlock. Query states again to refresh the UI.
         */
        clearAchievement (id: string): Promise<void>;
        /**
         * Reports observed state changes, not submission acknowledgements.
         * Returns an unsubscribe function. A fresh query may be needed after local resets.
         */
        onUpdated (callback: (state: AchievementState) => void): () => void;
    }

    // ────────────────────────────────────────────────────
    // IFriends
    // ────────────────────────────────────────────────────

    /**
     * @en Provides access to platform friends (list, avatar, groups, rich presence, overlay, invite).
     * @zh 提供平台好友功能的访问（列表、头像、分组、Rich Presence、Overlay、邀请）。
     */
    export class Friends {
        /** Current session user. Rejects NotReady if no user is available. */
        getLocalUser (): Promise<UserProfile>;
        /** Snapshot of confirmed friends. Empty lists are []; presence may be Unknown. */
        getFriends (): Promise<FriendInfo[]>;
        /** Null when no avatar is available. Size defaults to Medium; failures reject GsError. */
        getAvatar (userId: string, size?: AvatarSize): Promise<AvatarImage | null>;
        /** Optional friend grouping capability; unsupported providers reject NotSupported. */
        getGroups (): Promise<FriendGroup[]>;
        /** Optional capability. Resolves when the platform accepts the update, not remote delivery.
         * Keys and values are platform-defined; an empty value removes the key.
         */
        setRichPresence (key: string, value: string): Promise<void>;
        /** Clears current-user presence data; same completion boundary as setRichPresence. */
        clearRichPresence (): Promise<void>;
        /** Optional capability. Currently known value, or null when absent/unknown.
         * Does not guarantee a network refresh; Steam reads its local presence cache.
         */
        getRichPresence (userId: string, key: string): Promise<string | null>;
        /** Optional capability. Resolves when the request is dispatched, not when the page is visible. */
        openOverlay (dialog: OverlayDialog): Promise<void>;
        /** Opens an HTTP(S) URL in the platform overlay; same completion boundary as openOverlay. */
        openWebPage (url: string): Promise<void>;
        /** Join intent, not a completed connection. Returns an unsubscribe function. */
        onJoinRequested (callback: (request: JoinRequest) => void): () => void;
    }

    // ────────────────────────────────────────────────────
    // IRemoteStorage
    // ────────────────────────────────────────────────────

    /**
     * @en Provides access to platform cloud storage (read, write, delete files).
     * @zh 提供平台云存储的访问（读、写、删除文件）。
     */
    export class RemoteStorage {
        /** Writes a snapshot of the supplied bytes, including subarray offsets. SDK completion does not guarantee cloud synchronization. */
        writeFile (name: string, data: Uint8Array): Promise<void>;
        /** Returns owned bytes. Missing files reject with NotFound; empty files return an empty Uint8Array. */
        readFile (name: string): Promise<Uint8Array>;
        /** Writes UTF-8 using the engine TextEncoder. */
        writeText (name: string, text: string): Promise<void>;
        /** Reads UTF-8 using the engine TextDecoder; malformed input and BOM handling follow the runtime decoder. */
        readText (name: string): Promise<string>;
        /** Deleting an absent file succeeds. A conflicting transfer may reject with Busy. */
        deleteFile (name: string): Promise<void>;
        /** Queries metadata; an absent file returns null. Size is in bytes. */
        getFileInfo (name: string): Promise<FileInfo | null>;
        /** Queries a snapshot of platform-visible files; an empty store returns []. */
        listFiles (): Promise<FileInfo[]>;
        /** Optional quota query in bytes; rejects with NotSupported if unavailable. */
        getQuota (): Promise<QuotaInfo>;
    }

    // ────────────────────────────────────────────────────
    // IStats
    // ────────────────────────────────────────────────────

    /**
     * @en Provides access to platform statistics (set, get, store, reset).
     * @zh 提供平台统计的访问（设置、获取、存储、重置）。
     */
    export class Stats {
        /** Current session value; does not guarantee a network refresh.
         * Read failure rejects GsError, never a fabricated zero. Steam cannot distinguish a
         * missing stat, wrong type, or unavailable data, and reports PlatformError.
         */
        getInt (name: string): Promise<number>;
        /** Like getInt, for a floating-point stat. */
        getFloat (name: string): Promise<number>;
        /** Optional absolute assignment. Resolves when the platform accepts the change;
         * call flush() at a suitable save point. Success does not imply server persistence.
         * Integers must be JS-safe integers; Steam further requires signed 32-bit values.
         */
        setInt (name: string, value: number): Promise<void>;
        /** Optional absolute assignment of a finite number. Steam rounds to float32. */
        setFloat (name: string, value: number): Promise<void>;
        /** Optional signed increment, not absolute assignment. Same completion boundary as setInt.
         * Steam performs local read/add/write without yielding; not atomic across devices.
         * Out-of-range results reject InvalidArgument before changing the value.
         */
        incrementInt (name: string, delta: number): Promise<void>;
        /** Optional finite increment; same semantics as incrementInt with platform float precision. */
        incrementFloat (name: string, delta: number): Promise<void>;
        /** Requests submission of accepted changes. Resolves when the platform accepts the
         * submission request, not on server acknowledgement. Failure does not undo local changes.
         * Steam submits shared stats and achievement state together.
         */
        flush (): Promise<void>;
        /** Optional reset to platform defaults, preserving achievements unless explicitly requested.
         * Success means the reset was accepted, not server acknowledgement. Query again afterwards.
         */
        resetAll (options?: ResetStatsOptions): Promise<void>;
    }

    // ────────────────────────────────────────────────────
    // IUtils
    // ────────────────────────────────────────────────────

    /**
     * @en Provides access to platform utility functions.
     * @zh 提供平台工具函数的访问。
     */
    export enum DiagnosticLevel { Unknown = 0, Info, Warning, Error }
    export interface DiagnosticMessage { level: DiagnosticLevel; message: string; }
    export class Utils {
        /** Platform diagnostic message; not an operation result. Returns an unsubscribe function.
         * Availability is indicated by Services.hasModule(ServicesModule.Utils).
         */
        onDiagnostic (callback: (message: DiagnosticMessage) => void): () => void;
    }
}

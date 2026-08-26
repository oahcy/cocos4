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

import type { OnSuccessListener } from './core/callback';
import type { OnReadFileListener } from './core/callback';

export declare namespace gs {

    // ────────────────────────────────────────────────────
    // Callback / listener interfaces re-exported for users
    // ────────────────────────────────────────────────────

    export type OnSuccessListener = import('./core/callback').OnSuccessListener;
    export type OnReadFileListener = import('./core/callback').OnReadFileListener;

    // ────────────────────────────────────────────────────
    // Enums
    // ────────────────────────────────────────────────────

    /**
     * @en Platform services provider type.
     * @zh 平台服务提供商类型。
     */
    export enum ServicesProvider {
        /**
         * @en Null provider (no-op).
         * @zh 空提供商（无操作）。
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

    /**
     * @en Persona online state.
     * @zh 用户在线状态。
     */
    export enum PersonaState {
        Offline = 0,
        Online,
        Busy,
        Away,
        Snooze,
        LookingToTrade,
        LookingToPlay,
        Invisible,
    }

    /**
     * @en Avatar image size.
     * @zh 头像尺寸。
     */
    export enum AvatarSize {
        Small = 0,
        Medium,
        Large,
    }

    /**
     * @en Friend filter bit flags for getFriends(). Combine with `|`.
     *     Each platform maps these onto its own native constants.
     * @zh getFriends() 的好友筛选位标志，可用 `|` 组合。
     *     各平台内部映射到自己的原生常量。
     */
    export enum FriendFlags {
        None = 0,
        /**
         * @en Confirmed friends.
         * @zh 已确认的好友。
         */
        Immediate = 1,
        Blocked = 2,
        /**
         * @en Incoming friend request.
         * @zh 收到的好友请求。
         */
        FriendshipRequested = 4,
        /**
         * @en Outgoing friend request.
         * @zh 发出的好友请求。
         */
        RequestingFriendship = 8,
        ClanMember = 16,
        OnGameServer = 32,
        All = 0xFFFFFFFF,
    }

    /**
     * @en Platform overlay page for activateGameOverlay(). Not every platform
     *     supports every page; unsupported values are rejected by the native
     *     implementation with an error log.
     * @zh activateGameOverlay() 可打开的平台 Overlay 页面。并非所有平台都支持
     *     全部页面，不支持的值会被原生实现拒绝并记录错误日志。
     */
    export enum OverlayDialog {
        Friends = 0,
        Community,
        Players,
        Settings,
        OfficialGameGroup,
        Stats,
        Achievements,
    }

    /**
     * @en Friend info returned by getFriends().
     * @zh getFriends() 返回的好友信息。
     */
    export interface FriendInfo {
        userId: string;
        personaName: string;
        nickname: string;
        personaState: PersonaState;
    }

    /**
     * @en Avatar image with RGBA pixel data.
     * @zh 头像图片，包含 RGBA 像素数据。
     */
    export interface AvatarImage {
        width: number;
        height: number;
        data: ArrayBuffer;
    }

    /**
     * @en Friends group info.
     * @zh 好友分组信息。
     */
    export interface FriendsGroupInfo {
        groupId: number;
        groupName: string;
        members: string[];
    }

    /**
     * @en Achievement definition data.
     * @zh 成就定义数据。
     */
    export interface AchievementDefinition {
        AchievementId: string;
        DisplayName: string;
        Description: string;
    }

    /**
     * @en Achievement state data.
     * @zh 成就状态数据。
     */
    export interface AchievementState {
        AchievementId: string;
        Progress: number;
        UnlockTimeSec: number;
    }

    /**
     * @en File info returned by getFileList().
     * @zh getFileList() 返回的文件信息。
     */
    export interface FileInfo {
        FileName: string;
        FileSize: number;
    }

    /**
     * @en Cloud storage quota info.
     * @zh 云存储配额信息。
     */
    export interface QuotaInfo {
        TotalBytes: number;
        AvailableBytes: number;
    }

    /**
     * @en Stat integer result.
     * @zh 整数统计结果。
     */
    export interface StatIntResult {
        Success: boolean;
        Value: number;
    }

    /**
     * @en Stat float result.
     * @zh 浮点统计结果。
     */
    export interface StatFloatResult {
        Success: boolean;
        Value: number;
    }

    // ────────────────────────────────────────────────────
    // Entry point
    // ────────────────────────────────────────────────────

    /**
     * @en Get or create a game services instance.
     *     Returns null if no factory is registered for the given type.
     *     Call init() explicitly after restartAppIfNecessary check.
     * @zh 获取或创建游戏服务实例。
     *     未注册对应平台工厂时返回 null。
     *     需在 restartAppIfNecessary 检查后显式调用 init()。
     * @param servicesType - The provider type (default: Steam).
     * @param instanceName - Optional instance name for multiple instances.
     * @param instanceConfigName - Optional config name.
     */
    export function getServices (
        servicesType?: ServicesProvider,
        instanceName?: string,
        instanceConfigName?: string,
    ): Services | null;

    // ────────────────────────────────────────────────────
    // IGsServices
    // ────────────────────────────────────────────────────

    /**
     * @en Game services instance providing lifecycle management and sub-interface access.
     * @zh 游戏服务实例，提供生命周期管理和子接口访问。
     */
    export class Services {
        /**
         * @en Check if the app needs to be restarted through the platform launcher
         *     (e.g. the Steam client). Must be called BEFORE init().
         *     appId must be your published App ID, hardcoded into the shipped build —
         *     never sourced from a plain-text/user-editable file. (On Steam, the
         *     dev-only `steam_appid.txt` convenience file must NOT ship in the release
         *     build; it exists solely so `SteamAPI_InitEx()` can find an App ID when
         *     you run the exe directly without going through the Steam client.)
         *     Returns true: the launcher already relaunched the process correctly;
         *     quit immediately without calling init().
         *     Returns false: already running under the launcher (e.g. Steam sets the
         *     SteamAppId/SteamGameId environment variables before launching the game),
         *     so init() can proceed directly — init() itself takes no App ID.
         * @zh 检查应用是否需要通过平台启动器（如 Steam 客户端）重新启动。
         *     必须在 init() 之前调用。
         *     appId 必须是烧进发行包的真实已发布 App ID —— 绝不能来自纯文本、可被玩家
         *     修改的文件。（Steam 平台上，仅用于开发调试的 `steam_appid.txt` 文件不应
         *     出现在发行包中；它的作用只是让直接双击运行 exe 时 `SteamAPI_InitEx()`
         *     也能找到一个 App ID，绕过 Steam 客户端启动。）
         *     返回 true：启动器已经重新正确拉起了进程，应立即退出，不要调用 init()。
         *     返回 false：当前进程已经在启动器环境下运行（例如 Steam 客户端在拉起游戏
         *     前已设置好 SteamAppId/SteamGameId 环境变量），可以直接调用 init() ——
         *     init() 本身不需要也不接收 App ID 参数。
         *     Steam 请传入 number；string 留给未来非数字型 ID 的平台（如 Epic 的
         *     Product/Sandbox ID）—— Steam 不支持字符串，传入会失败并记录错误日志。
         * @param appId - Your published platform App ID (e.g. Steam AppID from Steamworks), hardcoded in the shipped build.
         *     Pass a number for Steam; string is reserved for future non-numeric
         *     platform IDs (e.g. Epic Online Services' Product/Sandbox ID) —
         *     Steam does not support a string and will fail with a logged error.
         */
        restartAppIfNecessary (appId: number | string): boolean;

        /**
         * @en Initialize the services instance. Must be called after restartAppIfNecessary check.
         * @zh 初始化服务实例。必须在 restartAppIfNecessary 检查之后调用。
         * @returns true if initialization succeeded.
         */
        init (): boolean;

        /**
         * @en Destroy the services instance and release resources.
         * @zh 销毁服务实例并释放资源。
         */
        destroy (): void;

        /**
         * @en Returns the active provider type.
         * @zh 返回当前激活的提供商类型。
         */
        getServicesProvider (): ServicesProvider;

        /**
         * @en Returns the achievements sub-interface.
         * @zh 返回成就子接口。
         */
        achievements (): Achievements;

        /**
         * @en Returns the friends sub-interface.
         * @zh 返回好友子接口。
         */
        friends (): Friends;

        /**
         * @en Returns the remote storage sub-interface.
         * @zh 返回云存储子接口。
         */
        remoteStorage (): RemoteStorage;

        /**
         * @en Returns the stats sub-interface.
         * @zh 返回统计子接口。
         */
        stats (): Stats;

        /**
         * @en Returns the utils sub-interface.
         * @zh 返回工具子接口。
         */
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
        /**
         * @en Query achievement definitions from the platform. Results cached locally.
         * @zh 从平台查询成就定义。结果缓存在本地。
         */
        queryAchievementDefinitions (): Promise<void>;

        /**
         * @en Query achievement states for the current account. Results cached locally.
         * @zh 查询当前账号的成就状态。结果缓存在本地。
         */
        queryAchievementStates (): Promise<void>;

        /**
         * @en Unlock an achievement. Calls storeStats internally.
         * @zh 解锁成就。内部调用 storeStats。
         * @param achievementId - The achievement API name.
         */
        unlockAchievements (achievementId: string): Promise<void>;

        /**
         * @en Clear an achievement (reset to locked).
         * @zh 清除成就（重置为未解锁）。
         * @param achievementId - The achievement API name.
         */
        clearAchievement (achievementId: string): Promise<void>;

        /**
         * @en Returns all cached achievement IDs (call queryAchievementDefinitions first).
         * @zh 返回所有缓存的成就 ID（需先调用 queryAchievementDefinitions）。
         */
        getAchievementIds (): string[];

        /**
         * @en Returns a cached achievement definition.
         * @zh 返回缓存的成就定义。
         * @param achievementId - The achievement API name.
         */
        getAchievementDefinition (achievementId: string): AchievementDefinition | null;

        /**
         * @en Returns a cached achievement state.
         * @zh 返回缓存的成就状态。
         * @param achievementId - The achievement API name.
         */
        getAchievementState (achievementId: string): AchievementState | null;

        /**
         * @en Registers a callback for achievement state updates (unlock, progress change).
         * @zh 注册成就状态更新的回调（解锁、进度变化）。
         * @returns A function to unregister this specific callback.
         */
        onAchievementStateUpdated (callback: (achievementId: string, progress: number, unlockTimeSec: number) => void): () => void;
    }

    // ────────────────────────────────────────────────────
    // IFriends
    // ────────────────────────────────────────────────────

    /**
     * @en Provides access to platform friends (list, avatar, groups, rich presence, overlay, invite).
     * @zh 提供平台好友功能的访问（列表、头像、分组、Rich Presence、Overlay、邀请）。
     */
    export class Friends {
        /**
         * @en Returns the current user's persona name.
         * @zh 返回当前用户的昵称。
         */
        getPersonaName (): string;

        /**
         * @en Returns the friend list filtered by flags.
         * @zh 按标志过滤返回好友列表。
         * @param friendFlags - Bitmask of FriendFlags (e.g. FriendFlags.Immediate).
         */
        getFriends (friendFlags: FriendFlags): FriendInfo[];

        /**
         * @en Request a friend's avatar image.
         * @zh 请求好友头像图片。
         * @param userId - The user's ID string.
         * @param size - Avatar size (Small/Medium/Large).
         */
        requestAvatar (userId: string, size: AvatarSize): Promise<AvatarImage>;

        /**
         * @en Returns all friends groups (tags).
         * @zh 返回所有好友分组（标签）。
         */
        getFriendsGroups (): FriendsGroupInfo[];

        /**
         * @en Set a rich presence key-value pair.
         * @zh 设置 Rich Presence 键值对。
         */
        setRichPresence (key: string, value: string): boolean;

        /**
         * @en Clear all rich presence data.
         * @zh 清除所有 Rich Presence 数据。
         */
        clearRichPresence (): void;

        /**
         * @en Get a friend's rich presence value by key.
         * @zh 通过 key 获取好友的 Rich Presence 值。
         */
        getFriendRichPresence (userId: string, key: string): string;

        /**
         * @en Activate the platform overlay to a specific page.
         * @zh 打开平台叠加层到指定页面。
         * @param dialog - Which overlay page to open.
         */
        activateGameOverlay (dialog: OverlayDialog): void;

        /**
         * @en Activate the Steam overlay to a web page.
         * @zh 打开 Steam 叠加层到指定网页。
         */
        activateGameOverlayToWebPage (url: string): void;

        /**
         * @en Register a callback for when a friend clicks "Join Game" via Rich Presence connect string.
         * @zh 注册回调，当好友通过 Rich Presence 的 connect 字符串点击"加入游戏"时触发。
         * @param callback - Receives friendId and the connect string set by the friend's game.
         * @returns A function to unregister this specific callback.
         */
        onGameRichPresenceJoinRequested (callback: (friendId: string, connectString: string) => void): () => void;
    }

    // ────────────────────────────────────────────────────
    // IRemoteStorage
    // ────────────────────────────────────────────────────

    /**
     * @en Provides access to platform cloud storage (read, write, delete files).
     * @zh 提供平台云存储的访问（读、写、删除文件）。
     */
    export class RemoteStorage {
        /**
         * @en Write data to a cloud file.
         * @zh 写入数据到云文件。
         * @param fileName - The file name.
         * @param data - The data to write (string).
         */
        writeFile (fileName: string, data: string): Promise<void>;

        /**
         * @en Read a cloud file.
         * @zh 读取云文件。
         * @param fileName - The file name.
         */
        readFile (fileName: string): Promise<string>;

        /**
         * @en Delete a cloud file.
         * @zh 删除云文件。
         * @param fileName - The file name.
         */
        deleteFile (fileName: string): Promise<void>;

        /**
         * @en Check if a file exists in cloud storage.
         * @zh 检查云存储中是否存在指定文件。
         * @param fileName - The file name.
         */
        fileExists (fileName: string): boolean;

        /**
         * @en Get the size of a cloud file in bytes.
         * @zh 获取云文件大小（字节）。
         * @param fileName - The file name.
         */
        getFileSize (fileName: string): number;

        /**
         * @en Get the total number of files in cloud storage.
         * @zh 获取云存储中的文件总数。
         */
        getFileCount (): number;

        /**
         * @en Get the list of all files in cloud storage.
         * @zh 获取云存储中的所有文件列表。
         */
        getFileList (): FileInfo[];

        /**
         * @en Get cloud storage quota info.
         * @zh 获取云存储配额信息。
         */
        getQuota (): QuotaInfo;
    }

    // ────────────────────────────────────────────────────
    // IStats
    // ────────────────────────────────────────────────────

    /**
     * @en Provides access to platform statistics (set, get, store, reset).
     * @zh 提供平台统计的访问（设置、获取、存储、重置）。
     */
    export class Stats {
        /**
         * @en Set an integer stat value.
         * @zh 设置整数统计值。
         * @param name - The stat API name.
         * @param value - The integer value.
         */
        setStatInt (name: string, value: number): Promise<void>;

        /**
         * @en Set a float stat value.
         * @zh 设置浮点统计值。
         * @param name - The stat API name.
         * @param value - The float value.
         */
        setStatFloat (name: string, value: number): Promise<void>;

        /**
         * @en Get an integer stat value.
         * @zh 获取整数统计值。
         * @param name - The stat API name.
         */
        getStatInt (name: string): StatIntResult;

        /**
         * @en Get a float stat value.
         * @zh 获取浮点统计值。
         * @param name - The stat API name.
         */
        getStatFloat (name: string): StatFloatResult;

        /**
         * @en Store stats to the platform server.
         * @zh 将统计数据存储到平台服务器。
         */
        storeStats (): Promise<void>;

        /**
         * @en Reset all stats (and optionally achievements).
         * @zh 重置所有统计数据（可选重置成就）。
         * @param achievementsToo - Whether to also reset achievements.
         */
        resetAllStats (achievementsToo: boolean): boolean;
    }

    // ────────────────────────────────────────────────────
    // IUtils
    // ────────────────────────────────────────────────────

    /**
     * @en Provides access to platform utility functions.
     * @zh 提供平台工具函数的访问。
     */
    export class Utils {
        /**
         * @en Set a warning message hook to receive platform warnings.
         * @zh 设置警告消息钩子以接收平台警告。
         * @param callback - Callback receiving severity and message.
         * @returns A function to unregister this specific callback.
         */
        onWarningMessage (callback: (severity: number, message: string) => void): () => void;
    }
}

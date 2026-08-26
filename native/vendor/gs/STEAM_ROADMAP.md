# Steam 集成路线图

架构参考：实际的组件注册表设计见 `STEAM_INTEGRATION.md`
（`native/vendor/gs/Framework/commons/*`、`native/vendor/gs/Platform/Steam/*`，TS 辅助层见 `vendor/gs/core/services.ts`）。
本文档只跟踪能力覆盖范围。

## 总览

| 能力 | 说明 | 状态 |
|------|------|------|
| **Achievements（成就）** | 查询成就定义/状态、解锁、清除 | ✅ 已完成 -- `IAchievements`（`Framework/Achievements.h`）+ `AchievementsSteam` + TS `AchievementsHelper` |
| **Stats（统计）** | 读写整数/浮点统计值、存储、重置 | ✅ 已完成 -- `IStats`（`Framework/Stats.h`）+ `StatsSteam` + TS `StatsHelper` |
| **Friends（好友）** | 好友列表、头像、Rich Presence、分组、Overlay 唤起、Rich Presence 加入事件 | ✅ 已完成 -- `IFriends`（`Framework/Friends.h`）+ `FriendsSteam` + TS `FriendsHelper` |
| **RemoteStorage（云存档）** | 云存档读/写/删除 + 文件/配额查询 | ✅ 已完成 -- `IRemoteStorage`（`Framework/RemoteStorage.h`）+ `RemoteStorageSteam` + TS `RemoteStorageHelper` |
| **Utils（工具）** | 仅警告消息钩子 | ✅ 已完成 -- `IUtils`（`Framework/Utils.h`）+ `UtilsSteam` + TS `UtilsHelper` |
| **User（用户）** | 身份与认证（SteamID、登录状态、Auth Ticket） | ⬜ 待实现（High Priority）-- 所有游戏都需要 |
| **Apps（应用）** | 应用/DLC 所有权、语言、构建信息 | ⬜ 待实现（High Priority）-- 以同步调用为主，实现成本低 |
| **Leaderboards（排行榜）** | 查找/创建排行榜、上传/下载分数 | ⬜ 待实现（High Priority）-- 复用已封装的 `ISteamUserStats`，无新增 SDK 依赖 |
| **Matchmaking / Lobby（匹配/大厅）** | 大厅创建/加入/搜索/数据/聊天 | ⬜ 待实现（Low Priority）-- 大多数游戏用自己的服务器撮合，只有需要 Steam 大厅时才用 |

已完成部分的底层实现细节（非独立能力接口）：

- **DLL 延迟加载** -- `SteamGsServices::onPreInitialize()` 中的 `LoadLibraryA` 探测（`Platform/Steam/SteamServicesModule.cpp`）
- **Tick** -- `GsServicesCommon::init()` 中惰性创建的 `cc::events::Tick` 监听器，每帧驱动 `SteamAPI_RunCallbacks()`

目前没有配套的示例项目 -- `STEAM_INTEGRATION.md` 里的用法示例是目前最接近的东西。

实现新能力时遵循现有模式（`Framework/Achievements.h` + `AchievementsCommon.h` +
`Platform/Steam/AchievementsSteam.h/.cpp` + `vendor/gs/core/services.ts` 中的 `AchievementsHelper`），
即每个能力是一个独立注册的 `GsComponent`，而不是一个平台专属的全局单例类。

---

## 接口细节

以下是尚未实现的四个能力（User / Apps / Leaderboards / Matchmaking）的具体接口/回调清单。

### User（用户）

身份与认证，所有游戏都需要。

| 接口 | 说明 |
|------|------|
| `getSteamId()` | 当前用户的 SteamID |
| `isLoggedOn()` | 是否已连接到 Steam 服务器 |
| `getPlayerSteamLevel()` | Steam 个人资料等级 |
| `getGameBadgeLevel(series, foil)` | 交易卡徽章等级 |
| `getAuthSessionTicket(callback)` | 用于游戏服务器验证的 Auth Ticket |
| `getAuthTicketForWebApi(identity, callback)` | 用于 Web API 验证的 Auth Ticket |
| `cancelAuthTicket(handle)` | 取消一个已获取的 Ticket |

回调：

| 回调 | 说明 |
|------|------|
| `GetAuthSessionTicketResponse_t` | Ticket 就绪通知 |
| `GetTicketForWebApiResponse_t` | Web API Ticket 就绪 |
| `MicroTxnAuthorizationResponse_t` | 微交易授权结果 |
| `SteamServersConnected_t` | 已连接到 Steam 服务器 |
| `SteamServersDisconnected_t` | 与 Steam 服务器连接断开 |

### Apps（应用）

应用所有权、语言、DLC -- 以同步调用为主，实现成本低。

| 接口 | 说明 |
|------|------|
| `isSubscribed()` | 用户是否拥有当前应用 |
| `isSubscribedApp(appId)` | 用户是否拥有指定 AppID |
| `isDlcInstalled(appId)` | 某 DLC 是否已拥有并安装 |
| `getDLCCount()` | DLC 条目总数 |
| `getDLCDataByIndex(index)` | 按索引获取某条 DLC 的 AppID/可用性/名称，用于枚举 DLC 列表（配合 `getDLCCount` 使用） |
| `getCurrentGameLanguage()` | 当前游戏所选语言 |
| `getAvailableGameLanguages()` | 所有支持的语言（逗号分隔） |
| `getAppBuildId()` | 当前构建 ID |
| `getAppOwner()` | 实际所有者的 SteamID（Family Sharing 场景下与当前用户不同） |
| `isSubscribedFromFamilySharing()` | 是否通过 Family Sharing 获得访问权限 |
| `isLowViolence()` | 低暴力地区检测 |
| `isCybercafe()` | 网吧授权检测 |
| `isVACBanned()` | VAC 封禁检测 |

回调：

| 回调 | 说明 |
|------|------|
| `DlcInstalled_t` | 某个 DLC 安装完成 |

### Leaderboards（排行榜）

与 Achievements/Stats 共用同一个 native 接口（`ISteamUserStats`），因此无需引入新的 SDK 依赖 --
绝大多数按分数/时间排名的游戏都会用到。建议与 `StatsSteam`（`Platform/Steam/StatsSteam.h`）放在一起实现，
而不是单独建一个平台文件，因为底层是同一个 `ISteamUserStats` 指针。

| 接口 | 说明 |
|------|------|
| `findOrCreateLeaderboard(name, sortMethod, displayType)` | 查找排行榜，不存在则创建（异步） |
| `findLeaderboard(name)` | 查找已存在的排行榜（异步） |
| `getLeaderboardName(handle)` / `getLeaderboardEntryCount(handle)` | 对已查找到的排行榜做同步元数据查询 |
| `getLeaderboardSortMethod(handle)` / `getLeaderboardDisplayType(handle)` | 同步元数据查询 |
| `uploadLeaderboardScore(handle, method, score, details)` | 上传当前用户的分数（异步） |
| `downloadLeaderboardEntries(handle, dataRequest, rangeStart, rangeEnd)` | 分页下载条目：全局 / 用户周边 / 好友（异步） |
| `downloadLeaderboardEntriesForUsers(handle, userIds)` | 下载指定用户集合的条目（异步） |
| `getDownloadedLeaderboardEntry(entriesHandle, index)` | 从下载结果集中读取一条记录（同步，仅在下一次下载前有效） |

回调：

| 回调 | 说明 |
|------|------|
| `LeaderboardFindResult_t` | `FindOrCreateLeaderboard`/`FindLeaderboard` 的结果 |
| `LeaderboardScoresDownloaded_t` | `DownloadLeaderboardEntries[ForUsers]` 的结果 |
| `LeaderboardScoreUploaded_t` | `UploadLeaderboardScore` 的结果 |

### Matchmaking / Lobby（匹配/大厅）

仅在需要实时组队（例如大逃杀式分组）时才需要。大多数游戏使用自己服务器端的匹配逻辑。

核心：

| 接口 | 说明 |
|------|------|
| `createLobby(type, maxMembers)` | 创建大厅 |
| `joinLobby(lobbyId)` | 加入大厅 |
| `leaveLobby(lobbyId)` | 离开大厅 |
| `requestLobbyList()` | 搜索大厅 |
| `getLobbyByIndex(index)` | 从搜索结果中按索引取大厅 ID |
| `getNumLobbyMembers(lobbyId)` | 成员数量 |
| `getLobbyMemberByIndex(lobbyId, i)` | 按索引取成员 |
| `getLobbyOwner(lobbyId)` | 获取大厅所有者 |

大厅数据：

| 接口 | 说明 |
|------|------|
| `setLobbyData(lobbyId, key, value)` | 设置大厅元数据（仅所有者可用） |
| `getLobbyData(lobbyId, key)` | 读取大厅元数据 |
| `setLobbyMemberData(lobbyId, key, value)` | 设置成员级数据（例如准备状态） |
| `getLobbyMemberData(lobbyId, userId, key)` | 读取成员级数据 |

搜索过滤：

| 接口 | 说明 |
|------|------|
| `addRequestLobbyListStringFilter(key, value, cmp)` | 按字符串过滤 |
| `addRequestLobbyListNumericalFilter(key, value, cmp)` | 按数值过滤 |
| `addRequestLobbyListResultCountFilter(max)` | 限制结果数量 |
| `addRequestLobbyListDistanceFilter(filter)` | 按距离过滤 |

管理：

| 接口 | 说明 |
|------|------|
| `setLobbyType(lobbyId, type)` | 修改大厅可见性 |
| `setLobbyMemberLimit(lobbyId, max)` | 修改最大成员数 |
| `setLobbyJoinable(lobbyId, joinable)` | 锁定/解锁大厅 |
| `setLobbyOwner(lobbyId, newOwner)` | 转让所有者 |
| `setLobbyGameServer(lobbyId, ip, port, serverId)` | 关联游戏服务器 |
| `requestLobbyData(lobbyId)` | 请求某个大厅的元数据 |
| `inviteUserToLobby(lobbyId, userId)` | 发送大厅邀请 |
| `sendLobbyChatMsg(lobbyId, data)` | 发送聊天消息 |

回调：

| 回调 | 说明 |
|------|------|
| `LobbyCreated_t` | 创建结果 |
| `LobbyEnter_t` | 加入结果 |
| `LobbyMatchList_t` | 搜索结果 |
| `LobbyDataUpdate_t` | 大厅/成员数据变更 |
| `LobbyChatUpdate_t` | 成员加入/离开/被踢 |
| `LobbyGameCreated_t` | 游戏服务器已设置 |
| `LobbyChatMsg_t` | 收到聊天消息 |
| `LobbyInvite_t` | 收到邀请 |

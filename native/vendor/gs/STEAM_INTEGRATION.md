# GS / Steam 实现说明

## 结构与所有权

对外继续按成就、好友、云存储、统计、工具分模块，Services 不包含模块业务方法。

```text
TS ServicesHelper → native GsServicesCommon → 模块 Facade
                              │                    │
                              └────→ GsSession ←───┘
                                         │
                              独占 GsPlatform 和六个 Backend
```

- 注册表按 Provider 保存当前服务；创建函数取代 ModuleInitializer/Factory 类层级。
- Services 持有 Session，并缓存六个模块 Facade。
- Facade 固定持有创建时的 Session，只在调用期间取得 Backend；不通过全局注册表重新寻找会话。
- Session 不持有 Services 或 Facade。Backend 使用 unique_ptr，既不引用计数，也不交给 JS。
- 为保持 JSB 名称兼容，对外门面仍叫 IAchievements、IFriends、IRemoteStorage、IStats、IUtils、IAccount，但它们已经是具体类。平台实现继承独立的 IAchievementsBackend 等内部接口。
- TS Helper 负责 Promise 与事件多订阅。初始化、关闭状态以 native Session 为准，不再维护第二份初始化标记。

## 文件职责

| 文件或目录 | 职责 |
|---|---|
| vendor/gs/index.ts / impl.ts | 对外声明与运行时导出 |
| vendor/gs/core/services.ts | TS Helper、每平台缓存、Promise 和事件分发 |
| Framework/commons/GsServices.h | 顶层服务契约，仅生命周期、Provider、模块 getter |
| Framework/commons/GsServicesCommon.h / Framework/GsServicesCommon.cpp | 服务门面及模块门面缓存 |
| Framework/commons/GsSession.h / Framework/GsSession.cpp | 会话状态、资源所有权、请求取消、延后关闭 |
| Framework/Achievements.h/.cpp 等 | 六个模块门面，各自包含本模块接口和转发 |
| Framework/backends/*Backend.h | 平台内部接口，没有 JS 对象或引用计数 |
| Framework/commons/GsCallback.h | 原生 callable、共享的一次完成状态与会话事件门控 |
| Framework/JsbConversions.h | JS 对象/函数与原生回调、数据之间的适配 |
| common/ScopedListener.*、JsUtils.* | JS 对象持有和调用，仅绑定适配层依赖 |
| Platform/Steam/*Steam.* | Steam SDK 功能实现 |
| Platform/Steam/SteamServicesModule.* | SteamPlatform 和 createSteamServices 创建函数 |
| native/tools/swig-config/gs/gs.i | JSB 生成输入，暴露服务及模块门面 |

已删除 GsComponent、GsComponentRegistry、AchievementsCommon，不再动态按 type_index 注册模块。

## 会话生命周期

```text
Created → Initializing → Ready → Closing → Closed
   ▲          │
   └── 失败 ──┘
Created / Initializing 也可关闭，经 Closing 进入 Closed。
```

1. getServices(provider) 返回当前未关闭服务，默认 Steam。无工厂返回 null。
2. await restartAppIfNecessary(appId) 仅在 Created 阶段调用。Steam 使用数字 App ID。
3. await init() 调用平台 initialize(modules, callback)，进入 Initializing 并持续 pump。完成通知在平台调用退出后处理，成功进入 Ready；并发 init 等待同一次初始化，Ready 下重复 init 直接成功。
4. 初始化失败由 Session 清理部分构建的模块并调用平台 shutdown，再回到 Created，允许重试。平台 shutdown 必须支持失败或未完成的初始化，取消 SDK 请求并停止访问 modules。
5. destroy() 立即进入 Closing，禁止新调用和后续事件分发。
6. 当前调用或平台回调尚未退出时，延后实际清理。最外层 Dispatch 退出后取消请求、注销平台回调、销毁 Backend，再关闭并释放平台上下文，进入 Closed。
7. Closed 永久失效。注册表下一次获取会创建新服务/新 Session；旧 Facade 仍指向旧 Session。

同平台仍在 Closing 时 getServices 返回 null，避免在旧 SDK 回调栈尚未退出时初始化新 SDK。可以在当前回调返回后的下一帧重新获取。

所有 native 操作、回调适配与关闭都在引擎线程执行。异线程 SDK 回调必须先投递到引擎线程再调用门面/派发事件。

### 回调中关闭

每个 Facade 方法及平台 pump 都持有 Session::Dispatch。JS 在成就事件里调用 destroy 时，只改变会话状态；Backend 和 SDK 在 SteamAPI_RunCallbacks 返回后才释放。会话事件门控阻止关闭请求之后同一轮 pump 的后续用户事件。

取消回调可能重入 Facade，因此 finishClose 有独立的执行保护，避免清理流程递归执行。

### 引擎清理

jsb_module_register.cpp 为每个脚本上下文创建一个 Tick listener，驱动注册表内的服务。它不属于单个 Service，因此关闭服务不会删除当前正在调用的 Tick listener。

before-cleanup hook 在 GC/Object::cleanup 前调用 registry.destroyServices()，统一关闭服务、释放 JS 回调并清空实例缓存。平台创建函数保留。清理过程中禁止回调再创建服务；新脚本上下文重建 TS 缓存和 Tick listener。

JS 仍持有 Facade 时只会延长已关闭 Session 空壳的寿命，不会延长 Backend 或 SDK 资源寿命。

## 回调与结果

- AsyncCallback 使用共享完成状态，成功、失败、取消最多执行一次；完成前先移走处理函数，避免重入覆盖状态。
- Session 跟踪未完成请求。关闭时以 GsErrorCode.Cancelled（Services closed）取消；迟到的 SDK 结果不会再次完成请求。完成和取消都释放成功/失败处理函数捕获的 JS 引用。
- EventDelegate 是持续监听。SessionGate 不反向持有 Session，避免形成引用环。关闭后不再分发，Backend shutdown 释放监听。
- JSB 的 JsbConversions 将 JS 回调转换为原生 callable，ScopedListener 的 root/incRef 只留在适配层。
- TS 异步操作在 Closing/Closed 时拒绝 Cancelled；模块 getter 抛 Cancelled。未初始化获取模块为 NotReady，Ready 会话中缺少模块为 NotSupported。原生模块 getter 仍以 null 表示不可用。

本轮保持现有业务结果语义：Stats.flush 和成就 unlock/clear 的成功表示 StoreStats 调用被接受，不代表服务器确认。云存储仍最多同时一个读、一个写；未加入请求队列；Steam 读写现有 60 秒等待超时。

## Steam 接入

启用 USE_VENDOR_STEAM，通用 USE_VENDOR_GS 由后端开关派生。通用 JSB 只生成和注册一次。JS 构建在该开关下将 vendor/gs/index.ts 替换为 impl.ts。

Windows SDK 文件：

```text
native/external/win64/include/steam/steam_api.h
native/external/win64/libs/steam_api64.lib
native/external/win64/libs/steam_api64.dll
```

SteamPlatform 负责 DLL 探测、SteamAPI_InitEx、创建六个 Backend、SteamAPI_RunCallbacks 和 SteamAPI_Shutdown。Backend shutdown 必须在 SDK shutdown 前注销 STEAM_CALLBACK / CCallResult。

开发运行保持 Steam 客户端启动，按项目原有方式配置 steam_appid.txt（测试 App ID 可使用现有 480 配置）。发行配置继续遵循项目自己的 App ID 和启动流程。Overlay 需要 Steam 启动环境，直接从编辑器运行不一定能显示。

```ts
const services = gs.getServices(gs.ServicesProvider.Steam);
if (services) {
    // 启动器检查应在游戏启动流程中先 await，并在返回 true 时退出当前进程。
    await services.init();
    const achievements = services.achievements();
    const definitions = await achievements.queryDefinitions();
    const states = await achievements.queryStates();
    services.destroy();
    // achievements 属于旧会话，不能继续使用。
    const next = gs.getServices(gs.ServicesProvider.Steam);
    await next?.init();
}
```

接入项目原有单参数 getServices 调用无需修改，但需使用配套的新 TS 与 native 代码重新构建。

## 后续接入 Epic 或新增能力

新增平台实现 GsPlatform 与所需模块 Backend，并通过 registerProvider(provider, createFunction) 注册。支持的模块填入 GsModules；不支持的模块保留空。还需补 SDK 构建配置、USE_VENDOR_GS 派生条件、编辑器 feature 和 TS override 条件。

Facade/TS API 不应加入平台专属参数。异步用户认证、用户切换和能力就绪状态属于后续功能，目前 init 仅代表现有平台会话准备完成，未实现通用账号系统。

新增能力只增加对应模块 Facade/Backend、Session 的明确成员、Services getter、JSB 配置和 TS Helper，不把业务方法加入顶层 Services。

## 业务边界

业务边界：大头像句柄 0 立即报告无头像，-1 才等待；等待超过 30 秒后在下一次平台 pump 中失败。头像队列最多 50 项，满时拒绝新请求，回调通知前先移出完成项。成就 queryDefinitions/queryStates 直接返回数组，互不依赖；不再提供缓存 getter。状态读取失败会拒绝整个查询，不返回部分结果。状态含 id、unlocked、progress、unlockedAt；未知进度及未知解锁时间为 null。修改或重置后可重新 queryStates 更新界面。云存档支持读取存在的空文件；getQuota 查询失败拒绝 Promise，不再伪装成零配额。StoreStats 的成功仍只表示 SDK 接受提交。

## 公共 API 迁移：第一批

成就接口已改为 queryDefinitions、queryStates、unlock、clearAchievement 和 onUpdated。返回数组由调用方持有；定义为 id/displayName/description，状态为 id/unlocked/progress/unlockedAt。unlockedAt 使用 Unix 秒。Steam 锁定成就查询无法提供通用进度时返回 null，不把未知进度伪装成 0。

异步成就操作失败统一 reject GsError；code 使用 GsErrorCode，provider 标明服务提供商，platformCode 为平台错误码字符串或 null。会话关闭为 Cancelled，未准备好为 NotReady，不支持为 NotSupported，非法 ID 为 InvalidArgument，不存在的成就为 NotFound。各 Backend 均显式返回 GsError，不通过解析消息猜测错误类别。

unlock/clearAchievement 的成功表示平台接受修改及提交请求，不承诺服务器持久化。失败也不保证此前的本地修改已回滚。onUpdated 是观察到的状态通知，不作为保存确认。clearAchievement 为可选能力，Backend 默认返回 NotSupported。

各模块接口已完成本轮迁移；保持现有 Session 所有权和分模块设计，不保留旧成就接口兼容层。

## 公共 API 迁移：第二批云存档

writeFile(name, Uint8Array) / readFile(name) 使用二进制数据；写入时复制所传视图的字节，读取返回独立 Uint8Array。writeText/readText 提供 UTF-8 便利接口，复用引擎 TextEncoder/TextDecoder；非法编码和 BOM 的处理遵循运行时解码器，不保证严格校验。

listFiles() 返回 Promise<FileInfo[]>，字段为 name/size（字节）；getFileInfo(name) 返回对象或 null；getQuota() 返回 totalBytes/availableBytes，不支持时拒绝为 NotSupported。旧同步元数据接口已移除。

读取不存在文件为 NotFound，空文件正常返回；删除不存在文件成功。Steam 最多同时一个读、一个写，冲突为 Busy；读写期间删除也返回 Busy。单文件读写限制为 100 MiB。写入完成或失败时释放暂存字节缓冲区。写入成功表示 SDK 异步写入完成，不保证其他设备已经同步。所有操作失败使用 GsError，会话关闭取消尚未完成的请求。

## 公共 API 迁移：第三批好友

当前用户查询已移到 Account.getUser()；getFriends() 返回已确认好友数组，字段为 userId/displayName/nickname/presence。移除 FriendFlags 和列表包装结构；当前接口不提供屏蔽用户、请求或群成员筛选。昵称不可用为 null，ID 为平台内不透明字符串。

PresenceState 使用 Unknown/Offline/Online/Away/Busy，不沿用 Steam 枚举数值。Steam 的 Snooze 映射 Away、LookingToTrade/LookingToPlay 映射 Online，未识别状态映射 Unknown。本地用户查询不以服务器在线连接作为前提。

getAvatar(userId, size?) 默认 Medium，返回独立 RGBA8 Uint8Array；无头像为 null，读取失败才 reject。大小是期望档位，实际尺寸以返回值为准。Steam 大头像等待仍为最多 50 个请求、30 秒超时，分别返回 Busy/Timeout。

getGroups() 是可选能力，返回 id/displayName/memberIds 数组；分组 ID 也是字符串。setRichPresence/clearRichPresence 返回 Promise<void>，成功表示平台接受本地更新；空字符串值删除对应键。getRichPresence(userId, key) 返回 string|null，Steam 读取当前缓存，不承诺网络刷新。富状态键和值的具体约定仍由平台配置决定。

openOverlay/openWebPage 返回 Promise<void>，表示已发出请求，不保证界面已经显示。Steam Overlay 未启用返回 NotReady。分组、富状态和 Overlay 的 Backend 默认实现返回 NotSupported。

onJoinRequested 传入 { userId, connectionString }，返回取消订阅函数；该事件表示加入意图，不代表游戏已建立连接。connectionString 是游戏定义的连接数据。所有查询与修改失败统一为 GsError，关闭时未完成请求取消；保留原 Session/Facade 所有权结构。

## 公共 API 迁移：第四批统计

getInt/getFloat 返回 Promise<number>，不再返回 Success/Value 包装，也不把读取失败作为零值。名称必须非空且不含 NUL。Steam GetStat 无法区分名称不存在、类型不匹配或数据未准备好，统一为 PlatformError；不根据错误消息猜测 NotFound。

setInt/setFloat 设置绝对值，incrementInt/incrementFloat 接收有符号增量，均返回 Promise<void>。整数参数须为 JS 安全整数；Steam Backend 限制最终值为 int32，浮点为有限 float32，写入前检查范围。浮点值会按平台精度舍入。Steam 增量在同一次引擎线程调用内读/加/写，不保证跨设备或服务器原子累加。

修改成功表示平台接受变更。flush() 请求提交，成功表示 SDK 接受请求，不是服务器保存确认；提交失败不回滚本地修改。Steam 的统计与成就共用提交状态，成就提交也可能提交统计。不要逐帧重复提交；在游戏保存点调用。

resetAll({ includeAchievements?: boolean }) 为可选重置能力，默认 false，明确传 true 才同时重置成就。成功表示平台接受重置，不保证服务器持久化，之后重新查询显示值。Backend 对不支持的修改、提交、重置返回 NotSupported，不伪造成功。

原 getStatInt/getStatFloat/setStatInt/setStatFloat/storeStats/resetAllStats 和 StatIntResult/StatFloatResult 已移除。调用方应 await 查询及操作，用 GsError 处理失败。

## 公共 API 迁移：第五批 Services / 工具

init() 返回 Promise<void>，失败为 GsError；Steam SDK 错误文本和初始化结果码透传，不再只返回 false。初始化失败保持 Created 可重试；Ready 下重复初始化成功；Closing/Closed 下拒绝 Cancelled。getState() 和 getServicesProvider() 可在关闭后读取，destroy() 保持幂等。

ServicesState 新增 Initializing（值为 4，原有枚举值不变）。Ready 只表示 SDK 会话建立，不表示用户在线、认证完成或平台数据已同步。平台 initialize 可以立即完成，也可保存回调，在后续引擎线程任务或 pump 中完成；Steam 仍立即完成。关闭时取消全部初始化等待者，失效的完成通知不会恢复旧会话。初始化回调只持有弱结果令牌，不持有 Session；平台 shutdown 必须取消自身的延迟工作，令牌不代替平台资源管理。账号身份和可选登录由独立 Account 模块提供；会话内不提供账号切换。

hasModule(ServicesModule.X) 查询当前会话是否提供模块，必须在初始化之后调用；它不保证模块内每项可选能力都支持。未支持操作继续返回 NotSupported。

restartAppIfNecessary(appId) 返回 Promise<boolean>：true 表示已请求重新启动，应结束当前进程；false 仅表示没有请求重启，不是认证或联网成功。必须在 init 前调用；不支持的平台拒绝 NotSupported。Steam 数字 ID 必须为非零 uint32，错误字符串 ID 不再被当作“不需要重启”。

utils().onDiagnostic(callback) 接收 { level, message }，返回取消函数。DiagnosticLevel 为 Unknown/Info/Warning/Error；Steam 原始 0/1 映射 Info/Warning。消息不是操作完成结果；逐个隔离监听器异常，支持分发中取消订阅。初始化后先判断 Utils 模块是否可用。

接入方需要 await 启动器检查和初始化；页面共享同一启动 Promise。启动失败应展示错误而不是继续获取模块；退出时同时关闭正在启动和已就绪的会话。需要重新生成绑定并构建原生工程。

事件每次订阅拥有独立的取消函数；同一个回调可独立订阅多次，重复取消旧订阅不会影响后续新订阅。公共调用使用 Promise/GsError，旧 OnSuccessListener/OnReadFileListener 类型和字符串错误兼容构造已移除。

## Account 模块

ServicesModule.Account（值为 5）与 services.account() 提供独立账号入口。旧 Friends.getLocalUser() 已移除，改用 account.getUser()，返回 UserProfile 或 null。null 表示没有本地用户身份；接口失败仍返回 GsError。用户身份与 SDK Ready、联网状态、游戏服务器认证相互独立，离线不等于退出登录。

account.login() 是可选的异步平台登录入口，成功返回 UserProfile；支持该能力的平台在已有用户时返回同一用户，并发登录返回 Busy。Session 统一跟踪和取消未完成请求，Backend.shutdown 必须取消底层登录任务并停止后续 SDK 访问。Steam 的身份由客户端提供，login() 返回 NotSupported；getUser() 读取客户端的用户 ID 和显示名，不启动登录界面。

会话内不切换用户。接入新平台时，账号改变应关闭原会话并重新创建，避免成就、统计、云存储及旧 Facade 混用用户数据。此模块尚不提供票据、令牌、游戏服务器认证或登录状态事件。

诊断钩子只复制消息到有界队列，SteamPlatform 在 SDK 调用返回后派发；监听器产生的新消息留到下一帧。队列最多保留 256 条，溢出消息仍输出原生日志但不进入 JS 队列；取消、更换监听器或关闭时丢弃旧消息。SessionGate 关闭后，尚未完成请求的成功与失败通知均转为 Cancelled。

Steam 云存储读写采用单调时钟的 60 秒等待期限，每约 0.5 秒检查，并优先处理 SDK 回调。超时返回 Timeout，但不代表底层操作被取消，也不保证写入未发生。不注销尚未完成的调用结果、不释放其写入数据；同方向读写及删除仍受 Busy 保护，直到实际回调或关闭会话。迟到的读回调仍调用 FileReadAsyncComplete 消费结果，但不再通知已超时请求。不会自动重试。若 SDK 始终不回调，当前会话的通道仍无法恢复；超时只保证调用方结束等待。

## 公共请求超时

Session.track(callback.pending(), timeout) 接受可选的毫秒时长，不传时保持无超时。模块门面指定策略：头像查询 30 秒，文件读写 60 秒。Session 使用单调时钟，每约 0.5 秒在平台 pump 返回后清理已完成请求并结算到期请求；已就绪的 SDK 回调优先。超时通知前先移出本批请求，支持回调重入；若其中一个回调关闭会话，剩余未完成请求返回 Cancelled。初始化与持续事件不参与这套请求计时。

Steam Backend 不再保存这些请求的截止时间。Friends 仅移除已结算的头像等待项（新请求入队前也清理），云存储继续持有调用结果与写入数据，直到实际完成或会话关闭。Timeout 仅结束调用方等待，不取消 SDK 工作，也不自动放开冲突请求。完成后的回调不再持有 JS 处理函数，迟到结果仍用于平台资源收尾。

Steam 的 UserStatsStored_t 由 StatsSteam 统一监听，AchievementsSteam 仅监听 UserAchievementStored_t。提交失败保留原生日志，并通过平台注入的 DiagnosticSink 进入现有诊断队列，在 SDK 调用退出后由 Utils 派发 Error 消息（包含 EResult）；InvalidParam 会提示重新查询服务器修正后的统计值。不会自动重试，也不会再次结算已成功的提交 Promise。该消息是统计和成就共用提交的诊断，不是单次 flush 的回执。

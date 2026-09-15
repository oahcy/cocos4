# GS / Steam 实现说明

## 结构与所有权

对外继续按成就、好友、云存储、统计、工具分模块，Services 不包含模块业务方法。

```text
TS ServicesHelper → native GsServicesCommon → 模块 Facade
                              │                    │
                              └────→ GsSession ←───┘
                                         │
                              独占 GsPlatform 和五个 Backend
```

- 注册表按 Provider 保存当前服务；创建函数取代 ModuleInitializer/Factory 类层级。
- Services 持有 Session，并缓存五个模块 Facade。
- Facade 固定持有创建时的 Session，只在调用期间取得 Backend；不通过全局注册表重新寻找会话。
- Session 不持有 Services 或 Facade。Backend 使用 unique_ptr，既不引用计数，也不交给 JS。
- 为保持 JSB 名称兼容，对外门面仍叫 IAchievements、IFriends、IRemoteStorage、IStats、IUtils，但它们已经是具体类。平台实现继承独立的 IAchievementsBackend 等内部接口。
- TS Helper 负责 Promise 与事件多订阅。初始化、关闭状态以 native Session 为准，不再维护第二份初始化标记。

## 文件职责

| 文件或目录 | 职责 |
|---|---|
| vendor/gs/index.ts / impl.ts | 对外声明与运行时导出 |
| vendor/gs/core/services.ts | TS Helper、每平台缓存、Promise 和事件分发 |
| Framework/commons/GsServices.h | 顶层服务契约，仅生命周期、Provider、模块 getter |
| Framework/commons/GsServicesCommon.h / Framework/GsServicesCommon.cpp | 服务门面及模块门面缓存 |
| Framework/commons/GsSession.h / Framework/GsSession.cpp | 会话状态、资源所有权、请求取消、延后关闭 |
| Framework/Achievements.h/.cpp 等 | 五个模块门面，各自包含本模块接口和转发 |
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
Created → Active → Closing → Closed
   │                           ▲
   └──────── close ────────────┘
```

1. getServices(provider) 返回当前未关闭服务，默认 Steam。无工厂返回 null。
2. restartAppIfNecessary(appId) 仅在 Created 阶段调用。Steam 使用数字 App ID。
3. init() 调用平台 initialize(modules)。初始化成功后进入 Active；重复 init 不重复打开 SDK。
4. 初始化失败仍保留 Created，允许重试，并清理部分构建的模块。GsPlatform 的失败实现必须自行回滚已取得的 SDK 资源。
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
- Session 跟踪未完成请求。关闭时以 Services closed 取消；迟到的 SDK 结果不会再次完成请求。完成和取消都释放成功/失败处理函数捕获的 JS 引用。
- EventDelegate 是持续监听。SessionGate 不反向持有 Session，避免形成引用环。关闭后不再分发，Backend shutdown 释放监听。
- JSB 的 JsbConversions 将 JS 回调转换为原生 callable，ScopedListener 的 root/incRef 只留在适配层。
- TS 普通调用在原生会话 Closing/Closed 时抛错。直接 native 门面的异步调用失败，同步调用返回默认结果；未支持模块的 getter 返回 null。

本轮保持现有业务结果语义：Stats.storeStats 和成就 unlock/clear 的成功表示 StoreStats 调用被接受，不代表服务器确认。云存储仍最多同时一个读、一个写；本轮没有加入队列或超时机制。

## Steam 接入

启用 USE_VENDOR_STEAM，通用 USE_VENDOR_GS 由后端开关派生。通用 JSB 只生成和注册一次。JS 构建在该开关下将 vendor/gs/index.ts 替换为 impl.ts。

Windows SDK 文件：

```text
native/external/win64/include/steam/steam_api.h
native/external/win64/libs/steam_api64.lib
native/external/win64/libs/steam_api64.dll
```

SteamPlatform 负责 DLL 探测、SteamAPI_InitEx、创建五个 Backend、SteamAPI_RunCallbacks 和 SteamAPI_Shutdown。Backend shutdown 必须在 SDK shutdown 前注销 STEAM_CALLBACK / CCallResult。

开发运行保持 Steam 客户端启动，按项目原有方式配置 steam_appid.txt（测试 App ID 可使用现有 480 配置）。发行配置继续遵循项目自己的 App ID 和启动流程。Overlay 需要 Steam 启动环境，直接从编辑器运行不一定能显示。

```ts
const services = gs.getServices(gs.ServicesProvider.Steam);
if (services && services.init()) {
    const achievements = services.achievements();
    await achievements.queryAchievementDefinitions();
    await achievements.queryAchievementStates();
    services.destroy();
    // achievements 属于旧会话，不能继续使用。
    const next = gs.getServices(gs.ServicesProvider.Steam);
    next?.init();
}
```

接入项目原有单参数 getServices 调用无需修改，但需使用配套的新 TS 与 native 代码重新构建。

## 后续接入 Epic 或新增能力

新增平台实现 GsPlatform 与所需模块 Backend，并通过 registerProvider(provider, createFunction) 注册。支持的模块填入 GsModules；不支持的模块保留空。还需补 SDK 构建配置、USE_VENDOR_GS 派生条件、编辑器 feature 和 TS override 条件。

Facade/TS API 不应加入平台专属参数。异步用户认证、用户切换和能力就绪状态属于后续功能，目前 init 仅代表现有平台会话准备完成，未实现通用账号系统。

新增能力只增加对应模块 Facade/Backend、Session 的明确成员、Services getter、JSB 配置和 TS Helper，不把业务方法加入顶层 Services。

## 业务边界

业务边界：大头像句柄 0 立即报告无头像，-1 才等待；等待超过 30 秒后在下一次平台 pump 中失败。头像队列最多 50 项，满时拒绝新请求，回调通知前先移出完成项。不存在或尚未缓存的成就返回 null；状态读取失败会清空状态缓存并报告失败，重新查询定义、成功 resetAllStats(true) 或保存失败也会清空状态缓存，需要重新 queryAchievementStates。云存档支持读取存在的空文件；getQuota 查询失败抛异常，不再伪装成零配额。StoreStats 的成功仍只表示 SDK 接受提交。

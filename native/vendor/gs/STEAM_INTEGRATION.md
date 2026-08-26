# GS (Game Services) / Steam Integration - Implementation Record

## Overview

Cocos Creator 引擎的平台 Game Services 抽象层，代号 `gs`，Steam 是其第一个（也是目前唯一完成的）平台实现，架构上预留了 `Epic` 等其他平台。
C++ feature flag 分两层：框架层 `USE_VENDOR_GS` / `CC_USE_VENDOR_GS`（派生值，由所有已启用的平台后端 OR 得出，`native/CMakeLists.txt:61-66`），平台层 `USE_VENDOR_STEAM` / `CC_USE_VENDOR_STEAM`（`native/CMakeLists.txt:58`）。分层的原因见文末「新增一个平台后端」。

当前实现是一套**接口分层 + 组件注册表**架构：每个能力（成就、好友、云存档...）是一个独立注册的组件，而不是一个个平台专属的全局单例类。下文按实际代码结构描述。

---

## 架构总览：Service 与 Component 的关系

**概念上**：`Service` 对应一个平台（`GsServicesType::Steam`/`Epic`/...），`Component` 是打包在某个 `Service` 内部的能力单元（成就、好友、云存档、统计、工具）。一个 Service 装多个 Component，Component 不跨 Service 共享。

```
                    GsServicesType（Null / Steam / Epic...）
                                  │
                                  ▼
                ┌───────────────────────────────────┐
                │  Service（IGsServices 实现）         │  ← 一个平台 = 一个 Service
                │  例：SteamGsServices                 │
                │                                     │
                │   ┌───────────────────────────┐     │
                │   │  Component（能力单元）        │     │  ← 多个能力打包进同一个 Service
                │   │                             │     │
                │   │  Achievements   Friends      │     │
                │   │  RemoteStorage  Stats        │     │
                │   │  Utils                       │     │
                │   └───────────────────────────┘     │
                └───────────────────────────────────┘
```

**运行期对象关系**：谁创建谁、谁持有谁（均已用 `rg` 核实，路径+行号见括号）。

```
GsServicesRegistry（进程级单例，Framework/commons/GsServicesRegistry.h:58）
  │
  │ addModuleInitializer(type, initializer)   ← 平台模块注册时挂进来
  │ registerFactories() → registerServicesFactory(type, factory)
  │
  │ getNamedServicesInstance(type, name, cfg)
  │ （GsServicesRegistry.h:90-121；同一 GsServicesType 下只允许一个实例存活，见 :108-114）
  ▼
IGsServicesFactory::create() ──创建──▶ Service 实例，例如 SteamGsServices
                                          │ （Platform/Steam/SteamServicesModule.h:94-99）
                                          │
                                          │ init() → registerComponents()
                                          │ （SteamServicesModule.h:77-85）
                                          ▼
                          GsComponentRegistry（每个 Service 实例各持有一份）
                          （Framework/commons/GsComponentRegistry.h:47）
                                          │
                                          │ registerComponent<Interface, Concrete>()
                                          │ 按 type_index(Interface) 存放（GsComponentRegistry.h:53-61）
                                          ▼
    ┌───────────────┬───────────────┬───────────────────┬────────────┬────────────┐
    ▼               ▼               ▼                    ▼            ▼
AchievementsSteam FriendsSteam  RemoteStorageSteam    StatsSteam   UtilsSteam
实现 IAchievements 实现 IFriends 实现 IRemoteStorage  实现 IStats  实现 IUtils
（均为 Component，位于 native/vendor/gs/Platform/Steam/）
```

`Service` 与 `Component` 的单例规则、生命周期细节见下文「接口层」小节，此处只强调一点扩展性：新增一个平台（如 Epic）= 实现一个新的 `IGsServices` 子类 + 向 `GsServicesRegistry` 注册工厂；新增一个能力（如 Matchmaking）= 定义新的能力接口 + 让各平台的 `registerComponents()` 里多注册一个 Component。两者是正交的扩展点，互不影响。

---

## 目录结构

```
vendor/gs/                              ← TS 层
├── index.ts                            ← 类型声明（declare namespace gs，USE_VENDOR_STEAM=OFF 时使用）
├── impl.ts                             ← 运行时绑定（USE_VENDOR_STEAM=ON 时使用，由 moduleOverrides 切换）
└── core/
    ├── enums.ts                        ← 挂 jsb.GsServicesType（Null/Steam/Epic）
    ├── callback.ts                     ← OnSuccessListener / OnReadFileListener
    └── services.ts                     ← Helper 包装层（GsServicesHelper + 各 XxxHelper）

native/vendor/gs/                       ← C++ 层
├── common/
│   ├── JsUtils.h/cpp                   ← callJSfunc / invokeJSfunc
│   └── ScopedListener.h/cpp            ← RAII listener 守卫（root/incRef ↔ unroot/decRef）
├── Framework/
│   ├── commons/
│   │   ├── GsTypes.h                   ← 跨接口共享的值类型（AppId / AccountId）
│   │   ├── GsServices.h                ← IGsServices 顶层接口 + GsServicesType 枚举
│   │   ├── GsServicesCommon.h/.cpp     ← GsServicesCommon 通用生命周期基类
│   │   ├── GsComponent.h               ← IGsComponent 生命周期接口 + GsComponent<T> CRTP mixin
│   │   ├── GsComponentRegistry.h       ← 按 type_index 索引的组件表
│   │   ├── GsServicesRegistry.h        ← 进程级单例，按 GsServicesType 缓存/创建 IGsServices 实例
│   │   └── GsCallback.h                ← AsyncCallback<Args...> / EventDelegate<Args...>
│   ├── Achievements.h / AchievementsCommon.h
│   ├── Friends.h
│   ├── RemoteStorage.h
│   ├── Stats.h
│   ├── Utils.h
│   └── JsbConversions.h                ← POD 结构体的手写 se::Value 转换
└── Platform/Steam/
    ├── SteamServicesModule.h/.cpp      ← SteamGsServices（继承 GsServicesCommon）+ 工厂/模块初始化
    ├── AchievementsSteam.h/.cpp
    ├── FriendsSteam.h/.cpp
    ├── RemoteStorageSteam.h/.cpp
    ├── StatsSteam.h/.cpp
    └── UtilsSteam.h/.cpp

native/tools/swig-config/gs/gs.i        ← SWIG 模块定义，`%module(target_namespace="jsb") gs`
```

---

## 接口层（平台无关）

- **`IGsServices`**（`native/vendor/gs/Framework/commons/GsServices.h`）：顶层生命周期/工厂接口。
  `init() / destroy() / tick() / getServicesProvider()` + 五个 `getXxxInterface()`（Achievements/Friends/RemoteStorage/Stats/Utils），静态工厂 `getServices(GsServicesType, instanceName, instanceConfigName)`。
- **`GsServicesCommon`**（`Framework/commons/GsServicesCommon.h/.cpp`）：通用基类。
  `init()` 顺序：若 `_isInitialized` 已为真直接返回 `true`（幂等）→ `onPreInitialize()` → 置 `_isInitialized = true` → `registerComponents()` → 遍历组件 `initialize()` → `postInitialize()` → 创建 `_tickListener`（绑定 `cc::events::Tick`，只有 init 成功才会创建，避免失败后残留监听器）。
  `destroy()`：若 `_isInitialized` 已为假直接返回（幂等）→ 销毁 `_tickListener` → 遍历组件 `shutdown()` → 清空组件表 → `onPostShutdown()` → 置 `_isInitialized = false`。
  `onPreInitialize()/onPostShutdown()/registerComponents()` 是平台子类需要重写的钩子。
  **注意**：`destroy()` 只是清空 `GsComponentRegistry` 自己持有的引用；如果 JS 侧的 Helper 对象仍持有某个组件的 `IntrusivePtr`，该组件实例会存活到对应 JS 对象被 GC 才真正释放（`shutdown()` 已经把它标记为不可用，调用会 fail-fast，但内存不会立刻释放）。而 `onPostShutdown()`（如 `SteamAPI_Shutdown()`）不受影响，`destroy()` 一调用就立即执行。
- **`GsComponent<T>`**（`Framework/commons/GsComponent.h`）：CRTP mixin，把某个能力接口（如 `IAchievements`）与生命周期接口 `IGsComponent`（`initialize/postInitialize/tick/shutdown`）组合在一起；提供 `getServices()` 和 `shutdown()/isShutdown()` 幂等保护。
- **`GsComponentRegistry`**（`Framework/commons/GsComponentRegistry.h`）：`unordered_map<type_index, IntrusivePtr<RefCounted>>`，`registerComponent<Interface,Concrete>(...)` 注册、`getComponent<Interface>()` 查找。
- **`GsServicesRegistry`**（`Framework/commons/GsServicesRegistry.h`）：进程级单例。各平台通过 `IGsModuleInitializer` 惰性注册 `IGsServicesFactory`；`getNamedServicesInstance(type, instanceName, instanceConfigName)` 按组合 key 缓存已创建的服务实例。
  **同一 `GsServicesType` 下限制只能存在一个实例**：若对应类型的实例表已非空（已有一个实例存活，无论其 `instanceName`/`instanceConfigName` 是否与本次请求一致），后续调用不会创建第二个实例，而是直接复用已存在的那个并打印 `CC_LOG_ERROR`（`GsServicesRegistry.h:102-114`）。该限制按 `GsServicesType` 分别隔离存储，因此不影响 Steam 与 Epic 同时共存；实例被 `removeNamedServicesInstance()` 移除后，该类型的实例表变空，下一次 `getNamedServicesInstance()` 才会真正创建新实例。这是因为 Steam（及大多数平台 SDK）底层是进程级全局单例状态（如 `SteamAPI_InitEx`/`SteamAPI_Shutdown`、`UtilsSteam::s_callback`），无法正确支持同一平台的多个并存实例。

### 已实现的五个能力接口

| 接口 | 文件 | 说明 |
|------|------|------|
| `IAchievements` | `Framework/Achievements.h` | 成就定义/状态查询（异步）、解锁/清除（异步）、状态变更事件 |
| `IFriends` | `Framework/Friends.h` | 好友列表、头像（异步）、Rich Presence、好友分组、Overlay 唤起、Rich Presence Join 事件 |
| `IRemoteStorage` | `Framework/RemoteStorage.h` | 云存档读/写/删除（异步）、文件信息/配额（同步） |
| `IStats` | `Framework/Stats.h` | 统计值读写（写为异步）、`storeStats`、`resetAllStats` |
| `IUtils` | `Framework/Utils.h` | 仅 `setWarningMessageHook`，用于接收 Steam SDK 的调试警告 |

**注意**：接口层尚未覆盖 User/Apps/Matchmaking，清单见文末「已完成能力一览」，详细计划见 `STEAM_ROADMAP.md`。

---

## Steam 平台实现

`SteamGsServices`（`native/vendor/gs/Platform/Steam/SteamServicesModule.h`）继承 `GsServicesCommon`：

- `onPreInitialize()`：Windows 上先 `LoadLibraryA("steam_api64.dll")` 探测（软失败，SDK/DLL 缺失不会崩溃），再 `SteamAPI_InitEx`。
- `onPostShutdown()`：`SteamAPI_Shutdown()`。
- `tick()`：`SteamAPI_RunCallbacks()` 后调用 `Super::tick()` 分发到各组件。
- `getServicesProvider()`：返回 `GsServicesType::Steam`。
- `registerComponents()`：注册 `AchievementsSteam / FriendsSteam / RemoteStorageSteam / StatsSteam / UtilsSteam` 五个组件。
- `restartAppIfNecessary(appId)`：`appId` 为 `AppId`（`uint32_t` 与 `std::string` 的 variant，供 Epic 等未来平台的字符串型 ID 复用同一接口）；Steam 实现仅支持数字分支，调用 Valve 的 `SteamAPI_RestartAppIfNecessary`，传入字符串会被拒绝（记录 `CC_LOG_ERROR` 并返回 `false`）。
- 另有 `SteamServicesFactory`（`IGsServicesFactory` 实现）、`SteamModuleInitializer`（`IGsModuleInitializer` 实现，向 `GsServicesRegistry` 注册工厂）、自由函数 `shutdownSteamModule()`（整个模块级别的收尾，与单个服务实例的 `destroy()` 不同）。

### 两种异步回调模式

Steam 官方 SDK 本身提供两种异步范式，`gs` 模块按需选用：

| 模式 | 用途 | 使用位置 |
|------|------|----------|
| `STEAM_CALLBACK` 宏 | 被动广播事件，Steam 主动推送，持续监听 | `AchievementsSteam`（`onUserStatsStored`/`onAchievementStored`）、`FriendsSteam`（`onAvatarImageLoaded`/`onGameRichPresenceJoinRequested`） |
| `CCallResult<T,P>` | 一次性请求-响应，绑定具体 `SteamAPICall_t` | `RemoteStorageSteam`（`FileWriteAsync`/`FileReadAsync`） |

对应到 `Framework/commons/GsCallback.h` 里的两种回调抽象：

- **`AsyncCallback<Args...>` / `OnComplete`**：一次性，`success()`/`failure(msg)` 后自动 reset，对应 Steam 的 `CCallResult` 场景或"发起请求→拿到一次结果"的语义。
- **`EventDelegate<Args...>`**：持久型，可反复 `invoke()`，对应 `STEAM_CALLBACK` 的持续广播场景。

两者都用 `#ifndef SWIG` 保护真实实现，`#ifdef SWIG` 分支给出简化别名，避免 SWIG 生成器解析 JS 引擎相关的实现细节。

### 生命周期安全设计

- `RemoteStorageSteam`：`writeFile`/`readFile` 若上一个同类调用的 `CCallResult::IsActive()` 为真，新调用直接失败（拒绝而非覆盖挂起回调），避免悬挂指针/资源泄漏；`shutdown()` 会 `Cancel()` 两个 `CCallResult` 并丢弃（不触发）挂起回调。
- `FriendsSteam`：`shutdown()` 明确先调用 `Super::shutdown()` 再反注册 STEAM_CALLBACK / 清空挂起队列——这是刻意选择的重入安全顺序。`_pendingAvatars` 队列上限 50，溢出丢弃最旧的请求。
- `UtilsSteam`：用静态 `s_callback` + 静态转发函数 `steamWarningHook`，因为 `ISteamUtils::SetWarningMessageHook` 只接受裸函数指针，无法绑定到实例方法；`shutdown()` 会同时清空 JS 监听器和 native hook（此时 Steam 本身仍然存活）。

---

## JS ↔ C++ 绑定

- SWIG 模块名是 **`gs`**（`native/tools/swig-config/gs/gs.i`），生成 `jsb_gs_auto.cpp/h`，入口函数 `register_all_gs()`。
- 在 `native/cocos/bindings/manual/jsb_module_register.cpp` 中，同样是**框架层外 + 平台层内**的两层 `#if`（与同文件里 `CC_USE_MIDDLEWARE` 包住 `CC_USE_SPINE`/`CC_USE_DRAGONBONES` 的写法一致）：
  - `#if CC_USE_VENDOR_GS` → `#include "cocos/bindings/auto/jsb_gs_auto.h"`，内层 `#if CC_USE_VENDOR_STEAM` → `#include "vendor/gs/Platform/Steam/SteamServicesModule.h"`（第 145-151 行）
  - `#if CC_USE_VENDOR_GS` → `se->addRegisterCallback(register_all_gs);`（第 242 行）—— 框架层，只能调一次
  - 内层 `#if CC_USE_VENDOR_STEAM` → `static cc::Gs::SteamModuleInitializer s_steamModuleInit; GsServicesRegistry::get().addModuleInitializer(GsServicesType::Steam, &s_steamModuleInit);`（第 245-246 行）—— 平台层，每个后端各注册一次到自己的 `GsServicesType` 桶。这一步才是 `SteamModuleInitializer` 真正被塞进 `GsServicesRegistry` 的地方；`registerFactories()`（即注册 `SteamServicesFactory`）本身仍是惰性的，在首次 `getServices()` 触发的 `ensureInitialized()` 里才执行。
- 所有 POD 结果结构体（`AchievementDefinition`、`FileInfo`、`QuotaInfo` 等）在 `.i` 文件里被 `%ignore`，改用 `Framework/JsbConversions.h` 手写的 `nativevalue_to_se`/`sevalue_to_native` 做转换（而不是让 SWIG 自动生成类包装）。
- `AsyncCallbackBase`/`EventDelegateBase`/`RefCounted`/`IGsServices::tick` 同样被 `%ignore`；所有抽象接口标了 `%nodefaultctor`/`%nodefaultdtor`。
- JS 回调对象的生命周期由 `cc::scopedListener`（`native/vendor/gs/common/ScopedListener.h/.cpp`）保护：构造时 `root()+incRef()`，析构/`reset()` 时 `unroot()+decRef()`，防止 native 持有回调期间 JS 对象被 GC 回收。
- `common/JsUtils.h` 提供两种调用 JS 的方式：`callJSfunc(obj, "onXxx", args...)`（按方法名调用 JS 对象上的方法，用于 `AsyncCallback` 的 `onSuccess`/`onFailure`）与 `invokeJSfunc(func, args...)`（直接调用 JS 函数对象，用于 `EventDelegate::invoke`）。

---

## 调用链路示例：从 TS 到 Steam SDK

用三个真实接口分别演示「同步风格的一次性回调」「跨帧的一次性回调」「持久事件广播」三种模式在各层之间的具体流转（均已用 `rg` 核实路径+行号）。

### 一、一次性异步回调（同帧）：`unlockAchievements`

```
TS  services.achievements().unlockAchievements('ACH_WIN')
    （vendor/gs/core/services.ts:117-125）
    │  return new Promise((resolve, reject) => {
    │    native.unlockAchievements('ACH_WIN', { onSuccess, onFailure })
    │  })
    ▼
SWIG 绑定（gs.i 生成的 jsb_gs_auto.cpp）
    │  { onSuccess, onFailure } 这个 JS 对象 → OnComplete（=AsyncCallbackBase）
    │  callback.bind(jsObj)：scopedListener root()+incRef()
    │  （common/ScopedListener.h）
    ▼
IAchievements::unlockAchievements(achievementId, OnComplete callback)
    │  （接口，Framework/Achievements.h:41）
    ▼
AchievementsSteam::unlockAchievements(...)（Platform/Steam/AchievementsSteam.cpp:81-106）
    │  stats->SetAchievement(id)   ← Steam SDK 同步调用
    │  stats->StoreStats()         ← Steam SDK 同步调用
    │  callback.success()          ← 仍在同一次 native 调用栈内，未跨帧
    ▼
AsyncCallbackBase::success()（Framework/commons/GsCallback.h:51-56）
    │  callJSfunc(_listener.get(), "onSuccess")   ← 触发后 _listener.reset()，一次性
    ▼
common/JsUtils.h:35-44 → invokeJsCallback(obj, "onSuccess", {})
    ▼
TS  调用 JS 对象的 onSuccess() → resolve() → await 处返回
```

要点：`SetAchievement`/`StoreStats` 本身是同步 SDK 调用，`callback.success()` 在 `unlockAchievements()` 这一次 native 调用里就执行完，Promise 几乎立即 resolve。

### 二、一次性异步回调（跨帧）：`writeFile`

```
TS  services.remoteStorage().writeFile('save.dat', data)
    （vendor/gs/core/services.ts:210 起）→ native.writeFile(fileName, data, {onSuccess, onFailure})
    ▼
RemoteStorageSteam::writeFile(...)（Platform/Steam/RemoteStorageSteam.cpp:53-76）
    │  SteamAPICall_t apiCall = SteamRemoteStorage()->FileWriteAsync(...)  ← 真异步 SDK 调用，立即返回 handle
    │  _pendingWriteCallback = std::move(callback)     ← OnComplete 先存起来，不触发
    │  _writeCallResult.Set(apiCall, this, &RemoteStorageSteam::onWriteComplete)
    ▼
   （writeFile() 调用返回，JS 侧 Promise 处于 pending，callback 尚未触发）

─────────────────────── 之后某一次引擎 tick ───────────────────────
SteamGsServices::tick()（Platform/Steam/SteamServicesModule.h:67-70）
    │  SteamAPI_RunCallbacks()   ← Steam 发现 apiCall 已完成，回调绑定的成员函数
    ▼
RemoteStorageSteam::onWriteComplete(pResult, bIOFailure)（RemoteStorageSteam.cpp:78-84）
    │  _pendingWriteCallback.success() / .failure(...)
    ▼
   （同「一」：AsyncCallbackBase::success/failure → callJSfunc → onSuccess/onFailure → resolve/reject）
```

要点：`writeFile` 的 `callback` 被存进成员变量 `_pendingWriteCallback`，真正触发点是**之后某次** `tick()` 里 `CCallResult` 回调 `onWriteComplete`，Promise 真正跨帧悬挂等待。

### 三、持久事件广播：`onAchievementStateUpdated`

```
TS  const unsubscribe = services.achievements().onAchievementStateUpdated(cb)
    （vendor/gs/core/services.ts:155-174）
    │  _achievementListeners.add(cb)      ← 多个 JS 订阅者共享一个 Set
    │  若是第一个订阅者：
    │    native.setOnAchievementStateUpdated(dispatchFn)
    │    dispatchFn = (achId, prog, time) => { for (cb of listeners) cb(achId, prog, time) }
    ▼
IAchievements::setOnAchievementStateUpdated(OnAchievementStateUpdated callback)
    │  （接口，Framework/Achievements.h:48）
    ▼
AchievementsCommon::setOnAchievementStateUpdated(...)（Framework/AchievementsCommon.h:41-44）
    │  _onUpdatedCallback = std::move(callback)   ← EventDelegate，不会自动 reset
    │  callback.bind(dispatchFnObj) 同样走 scopedListener root()+incRef()
    ▼
   （setOnAchievementStateUpdated() 调用返回，此时只是"挂号"监听，尚未发生任何事件）

──────────────── Steam 客户端在任意时刻主动推送（与 JS 调用解耦） ────────────────
STEAM_CALLBACK(AchievementsSteam, onUserStatsStored, UserStatsStored_t, _cbUserStatsStored)
    │  （声明于 Platform/Steam/AchievementsSteam.h:31，构造时绑定于 :15）
    │  由 tick() 里的 SteamAPI_RunCallbacks() 驱动触发
    ▼
AchievementsSteam::onAchievementStored(pCallback)（AchievementsSteam.cpp:155 起）
    │  更新本地缓存状态后调用：
    ▼
AchievementsCommon::notifyAchievementUpdated(...)（Framework/AchievementsCommon.h:56-60）
    │  _onUpdatedCallback.invoke(achievementId, progress, unlockTimeSec)
    ▼
EventDelegate<Args...>::invoke(...)（Framework/commons/GsCallback.h:102-106）
    │  invokeJSfunc(_listener.get(), args...)   ← 不会 reset，可反复触发
    ▼
common/JsUtils.h:50-60 → func->call(args, nullptr) → 直接调用 TS 侧 dispatchFn(achId, prog, time)
    ▼
TS  dispatchFn 遍历 _achievementListeners，逐个调用每个订阅者 cb(achId, prog, time)
```

要点：
- 广播的触发时机与 JS 调用**完全解耦**：`onAchievementStateUpdated` 只是登记监听（语义差异见上文「两种异步回调模式」），真正触发由 Steam 客户端在任意一次 `tick()` 期间通过 `STEAM_CALLBACK` 推送决定。
- 取消订阅：`unsubscribe()` → `_achievementListeners.delete(cb)` → 集合为空时 `native.setOnAchievementStateUpdated(null)` → `AchievementsCommon` 侧 `_onUpdatedCallback.reset()`，解绑 `scopedListener`（unroot+decRef），dispatch 函数可被 GC。

---

## TS 层封装

```typescript
import { gs } from 'cc';

const services = gs.getServices(gs.ServicesProvider.Steam);
if (services && services.init()) {
    // 同步 API
    services.friends().getPersonaName();
    services.stats().getStatInt('wins');

    // 异步 API（Promise）
    await services.achievements().unlockAchievements('ACH_WIN');
    await services.remoteStorage().writeFile('save.dat', jsonData);

    // 事件订阅（多订阅者共享一个 native 回调槽，返回取消订阅函数）
    const unsubscribe = services.achievements().onAchievementStateUpdated(
        (achievementId, progress, unlockTimeSec) => { /* ... */ },
    );

    // 关闭
    services.destroy();
}
```

- **`index.ts`**：纯类型声明（`export declare namespace gs {...}`），零运行时开销，`USE_VENDOR_STEAM=OFF` 时使用。
- **`impl.ts`**：运行时绑定。`gs.ServicesProvider = jsb.GsServicesType`，同理转出 `gs.PersonaState/AvatarSize/FriendFlags/OverlayDialog`；`gs.getServices = createServices`；`gs.Services/Achievements/RemoteStorage/Stats/Utils` 分别指向 `core/services.ts` 里的 `GsServicesHelper`/`XxxHelper` 类。构建时由 `cc.config.json` 的 `moduleOverrides` 根据 `USE_VENDOR_STEAM` 在 `index.ts`/`impl.ts` 间切换。
- **`core/enums.ts`**：side-effect 模块，仅当 `JSB && jsb.IGsServices` 存在时挂枚举值。SWIG 不会把 gs 的枚举注册进 `jsb` 命名空间，因此 `GsServicesType`/`PersonaState`/`AvatarSize`/`FriendFlags`/`OverlayDialog` 的数值都在这里定义，必须与 `Framework/Friends.h`、`commons/GsServices.h` 里的 C++ 枚举保持一致。
- **`core/callback.ts`**：`OnSuccessListener`（`onSuccess()/onFailure?(msg)`）、`OnReadFileListener`（`onSuccess(data)/onFailure?(msg)`）。
- **`core/services.ts`**：核心 Helper 层。
  - `GsHelperBase` + 共享的 `ServicesLifecycle` token：`GsServicesHelper.destroy()` 执行后翻转标志，所有已发出的 helper 实例（即便被用户额外持有一份引用）后续调用立刻抛异常，防止 use-after-destroy。
  - **多订阅者复用单一 native 回调槽**：`AchievementsHelper.onAchievementStateUpdated`、`UtilsHelper.onWarningMessage`、`FriendsHelper.onGameRichPresenceJoinRequested` 内部用 `Set<callback>` 扇出到多个 JS 订阅者，但只在首次订阅时向 native 注册一次监听（`setOnXxx`），最后一个取消订阅时才向 native 传 `null` 清空。
  - `GsServicesHelper` 的 `achievements()/friends()/remoteStorage()/stats()/utils()` 惰性创建并缓存对应 Helper；`destroy()` 会先反注册所有事件（`_unbindAllEvents()`）再销毁 native 对象，并把自身从模块级 `_servicesCache` 中移除以允许之后重新创建。
  - `createServices(servicesType, instanceName, instanceConfigName)`：按 `${type}_${instanceName}_${instanceConfigName}` 缓存 key 复用同一个 `GsServicesHelper`，避免重复创建。

---

## 使用前准备

### 1. 放置 Steamworks SDK 文件

```
native/external/win64/
├── include/steam/          ← Steamworks SDK 头文件（steam_api.h, isteamuserstats.h, isteamfriends.h, isteamremotestorage.h, isteamutils.h ...）
└── libs/
    ├── steam_api64.dll
    └── steam_api64.lib
```

### 2. CMake 配置

```bash
cmake -DUSE_VENDOR_STEAM=ON ..
```

对应 `native/CMakeLists.txt` 中的开关（以下行号均已用 `rg -n "USE_VENDOR_GS|USE_VENDOR_STEAM" native/CMakeLists.txt` 核实，非估算）。注意区分**框架层**（`USE_VENDOR_GS`，平台无关，任一后端开启即编译）与**平台层**（`USE_VENDOR_STEAM`，仅 Steam 实现）：

| 行 | 开关 | 内容 |
|---|---|---|
| 58 | `USE_VENDOR_STEAM` | `cc_set_if_undefined(... OFF)`，唯一由用户/编辑器设置的 gs 开关 |
| 61-66 | `USE_VENDOR_GS` | 由平台后端 OR 派生（当前只有 Steam 一项）。**不要**给它加 `cc_set_if_undefined` —— 它是派生值，用户单独打开它只会编出一个没有任何后端的框架 |
| 854-876 | `USE_VENDOR_GS` | `cocos_source_files(vendor/gs/common/... vendor/gs/Framework/...)`，共 19 个文件 |
| 878-893 | `USE_VENDOR_STEAM` | `cocos_source_files(vendor/gs/Platform/Steam/...)`，共 12 个文件 |
| 1207-1209 | `USE_VENDOR_GS` | `cc_gen_swig_files(.../tools/swig-config/gs ${SWIG_OUTPUT})` |
| 2402-2407 | `USE_VENDOR_GS` | `cocos_source_files(NO_WERROR NO_UBUILD ${SWIG_OUTPUT}/jsb_gs_auto.cpp/.h)` |
| 3510-3511 | 两者 | 预处理器定义 `CC_USE_VENDOR_GS` / `CC_USE_VENDOR_STEAM` |

前两个块合起来覆盖 `native/vendor/gs/` 下全部 31 个 `.h`/`.cpp`，无遗漏也无失效条目。

1207 和 2402 两处**必须**是 `USE_VENDOR_GS` 这一个条件而不是多个平台的独立 `if`：`cc_gen_swig_files` 对同一个 `jsb_gs_auto.cpp` 调两次会重复生成，`register_all_gs` 注册两次会直接触发 `ScriptEngine::addRegisterCallback` 里的去重 `assert`（`native/cocos/bindings/jswrapper/napi/ScriptEngine.cpp:199`）。

### 3. 运行时

项目根目录需要 `steam_appid.txt` 文件（内含 App ID 数字），Steam 客户端需运行。

### 4. Steam Overlay 注意事项

Overlay 内嵌显示依赖 Steam 客户端启动游戏进程时的注入（`GameOverlayRenderer64.dll` hook 渲染管线）。从编辑器直接运行 build 出来的 exe 时，`activateGameOverlay`/`activateGameOverlayToWebPage` 只会打开外部窗口而非内嵌 Overlay，这是预期行为而非 bug；测试 Overlay 内嵌效果需要通过 Steam 客户端"添加非 Steam 游戏"的方式启动可执行文件。

### 5. 验证步骤

1. CMake 配置通过（无未找到的库错误）
2. SWIG 生成 `jsb_gs_auto.cpp/h` 无错误
3. 编译通过（无链接错误）
4. 运行时 `gs.getServices(gs.ServicesProvider.Steam)!.init()` 返回 `true`
5. `services.friends().getPersonaName()` 返回有效昵称

---

## 已完成能力一览

详见 `STEAM_ROADMAP.md`。简要摘要：Achievements / Friends / RemoteStorage / Stats / Utils 五个能力接口 + Steam 具体实现已完成；User（登录/身份/Auth Ticket）、Apps（DLC/订阅）、Matchmaking（大厅）尚未实现。

---

## 新增一个平台后端（如 Epic / EOS）

开关已经按「框架层 / 平台层」分好，所以接入新后端是**纯增量**，不需要改动任何现有条件表达式的语义。

**注意各层的平台无关性**：`gs.i` 的 7 个 `%include`（第 72-79 行）全部指向 `Framework/`，零 `Platform/Steam/*`；`exports/gs.ts`、`vendor/gs/index.ts`、`vendor/gs/impl.ts`、`vendor/gs/core/services.ts` 也都只认 `IGsServices` 接口。因此**只要新后端不引入新的能力接口，`gs.i` 和整个 TS 层都不用改**，`register_all_gs()` 生成出来的绑定面已经能同时服务多个平台。

### C++ / CMake（`native/CMakeLists.txt`）

1. 第 58 行附近加一行 `cc_set_if_undefined(USE_VENDOR_EPIC OFF)`。
2. 第 62 行的派生条件改成 `if(USE_VENDOR_STEAM OR USE_VENDOR_EPIC)`。**这是唯一需要修改的既有条件**。
3. 第 878 行的 Steam 源码块后面，照样式新增一个 `if(USE_VENDOR_EPIC) cocos_source_files(vendor/gs/Platform/Epic/...) endif()`。
4. 第 3511 行附近照样式加一行 `CC_USE_VENDOR_EPIC` 的预处理器定义。

第 854 / 1207 / 2402 三处**不用动** —— 它们已经是框架层条件。

### 绑定注册（`native/cocos/bindings/manual/jsb_module_register.cpp`）

在两个 `#if CC_USE_VENDOR_GS` 块内部，各加一个与 `#if CC_USE_VENDOR_STEAM` 并列的 `#if CC_USE_VENDOR_EPIC` 分支：include 平台模块头、`addModuleInitializer(GsServicesType::Epic, &s_epicModuleInit)`。**不要**新增第二个 `addRegisterCallback(register_all_gs)`。

### 编辑器配置

1. `editor/engine-features/render-config.json`：照 `gs-steam`（第 389-398 行）的样式新增一个平级 feature（如 `gs-epic`，`cmakeConfig: "USE_VENDOR_EPIC"`）。

   **用平级 feature 而不是 `options` 组**：`options` 语义是互斥单选（`physics` / `spine` 那样），而 `GsServicesRegistry` 的单实例限制是按 `GsServicesType` 分桶存的（`GsServicesRegistry.h:102-114`），架构上支持多平台共存（EOS 常与 Steam 发行并用而非替代）。另外 `IFeatureGroup`（`editor/engine-features/types.ts:123-125`）继承的 `BaseItem` 里没有 `cmakeConfig` / `isNativeModule` / `envCondition`，组本身承载不了共享开关，用组也拿不到任何好处。

2. `cc.config.json`：照 `gs-steam`（第 206-211 行）的样式新增 feature 映射。注意 feature ID 与 module ID 是两个独立命名空间（`cc.config.schema.json:16` "Keys are feature IDs"，而 `modules` 里的 ID 是 `/exports/` 下的相对路径），所以新 feature 的 `modules` 仍然是 `["gs"]`（TS 侧只有一份平台无关导出，见 `spine-3.8` / `spine-4.2` 共用 `modules: ["spine"]` 的同构先例），只有 `intrinsicFlags` 换成新平台的 flag。

3. `cc.config.json` 第 509-515 行那条 `moduleOverrides` 条目的 `test`（第 510 行）要补成 OR：`context.buildTimeConstants.USE_VENDOR_STEAM || context.buildTimeConstants.USE_VENDOR_EPIC`。

   **这是最容易漏的一处**：它决定是否把纯类型声明 `vendor/gs/index.ts` 换成运行时实现 `vendor/gs/impl.ts`。漏了的话「只开 Epic」的构建会静默拿到零运行时的 `index.ts`，`gs.getServices` 直接是 `undefined`，而且不报任何编译错误。

4. `vendor/gs/core/enums.ts`：核对 `GsServicesType` 里新平台的枚举数值与 `Framework/commons/GsServices.h` 一致（SWIG 不把 gs 枚举注册进 `jsb`，数值全靠这里手写，见上文「TS 层封装」）。

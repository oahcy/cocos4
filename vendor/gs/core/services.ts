/****************************************************************************
 Copyright (c) 2025 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

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
****************************************************************************/

import type { OnSuccessListener, OnReadFileListener } from './callback';

declare const jsb: any;

// ────────────────────────────────────────────────────
// Data types matching C++ structs in Achievements.h
// ────────────────────────────────────────────────────

export interface AchievementDefinition {
    AchievementId: string;
    DisplayName: string;
    Description: string;
}

export interface AchievementState {
    AchievementId: string;
    Progress: number;
    UnlockTimeSec: number;
}

// ────────────────────────────────────────────────────
// Lifecycle guard — shared by the services instance and
// every helper it hands out, so destroying the instance
// invalidates even copies of a helper the user kept.
// ────────────────────────────────────────────────────

class ServicesLifecycle {
    private _destroyed = false;

    markDestroyed (): void {
        this._destroyed = true;
    }

    assertAlive (who: string): void {
        if (this._destroyed) {
            throw new Error(`[gs] ${who} has been destroyed. Do not keep using it after GsServicesHelper.destroy() — fetch a fresh services instance via createServices()`);
        }
    }
}

// Base class for the interface wrappers below. Each wrapper is a thin,
// transparent pipe over one native interface; the lifecycle token makes sure a
// pipe that outlives its services fails loudly instead of forwarding to a
// shut-down native object.
class GsHelperBase {
    protected readonly _native: any;
    private readonly _lifecycle: ServicesLifecycle;
    private readonly _name: string;

    constructor (native: any, lifecycle: ServicesLifecycle, name: string) {
        this._native = native;
        this._lifecycle = lifecycle;
        this._name = name;
    }

    protected assertAlive (): void {
        this._lifecycle.assertAlive(this._name);
    }
}

// ────────────────────────────────────────────────────
// AchievementsHelper — wraps jsb.IAchievements
// ────────────────────────────────────────────────────

export class AchievementsHelper extends GsHelperBase {
    private _achievementListeners = new Set<(achievementId: string, progress: number, unlockTimeSec: number) => void>();
    private _nativeListenerBound = false;

    constructor (native: any, lifecycle: ServicesLifecycle) {
        super(native, lifecycle, 'AchievementsHelper');
    }

    queryAchievementDefinitions (): Promise<void> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.queryAchievementDefinitions({
                onSuccess: () => resolve(),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    queryAchievementStates (): Promise<void> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.queryAchievementStates({
                onSuccess: () => resolve(),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    unlockAchievements (achievementId: string): Promise<void> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.unlockAchievements(achievementId, {
                onSuccess: () => resolve(),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    clearAchievement (achievementId: string): Promise<void> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.clearAchievement(achievementId, {
                onSuccess: () => resolve(),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    getAchievementIds (): string[] {
        this.assertAlive();
        const result = this._native.getAchievementIds();
        return result ? result.AchievementIds : [];
    }

    getAchievementDefinition (achievementId: string): AchievementDefinition | null {
        this.assertAlive();
        const result = this._native.getAchievementDefinition(achievementId);
        return result ? result.Definition : null;
    }

    getAchievementState (achievementId: string): AchievementState | null {
        this.assertAlive();
        const result = this._native.getAchievementState(achievementId);
        return result ? result.State : null;
    }

    onAchievementStateUpdated (callback: (achievementId: string, progress: number, unlockTimeSec: number) => void): () => void {
        this.assertAlive();
        this._achievementListeners.add(callback);

        if (!this._nativeListenerBound) {
            this._nativeListenerBound = true;
            this._native.setOnAchievementStateUpdated((achId: string, prog: number, time: number) => {
                for (const cb of this._achievementListeners) {
                    cb(achId, prog, time);
                }
            });
        }

        return () => {
            this._achievementListeners.delete(callback);
            if (this._achievementListeners.size === 0) {
                this._nativeListenerBound = false;
                this._native.setOnAchievementStateUpdated(null);
            }
        };
    }

    /**
     * @internal Called by GsServicesHelper during shutdown
     */
    _unbindAllEvents (): void {
        this._achievementListeners.clear();
        this._nativeListenerBound = false;
        this._native.setOnAchievementStateUpdated(null);
    }
}

// ────────────────────────────────────────────────────
// Data types matching C++ structs in RemoteStorage.h
// ────────────────────────────────────────────────────

export interface FileInfo {
    FileName: string;
    FileSize: number;
}

export interface QuotaInfo {
    TotalBytes: number;
    AvailableBytes: number;
}

// ────────────────────────────────────────────────────
// RemoteStorageHelper — wraps jsb.IRemoteStorage
// ────────────────────────────────────────────────────

export class RemoteStorageHelper extends GsHelperBase {
    constructor (native: any, lifecycle: ServicesLifecycle) {
        super(native, lifecycle, 'RemoteStorageHelper');
    }

    writeFile (fileName: string, data: string): Promise<void> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.writeFile(fileName, data, {
                onSuccess: () => resolve(),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    readFile (fileName: string): Promise<string> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.readFile(fileName, {
                onSuccess: (data: string) => resolve(data),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    deleteFile (fileName: string): Promise<void> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.deleteFile(fileName, {
                onSuccess: () => resolve(),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    fileExists (fileName: string): boolean {
        this.assertAlive();
        return this._native.fileExists(fileName);
    }

    getFileSize (fileName: string): number {
        this.assertAlive();
        return this._native.getFileSize(fileName);
    }

    getFileCount (): number {
        this.assertAlive();
        return this._native.getFileCount();
    }

    getFileList (): FileInfo[] {
        this.assertAlive();
        const result = this._native.getFileList();
        return result ? result.Files : [];
    }

    getQuota (): QuotaInfo {
        this.assertAlive();
        return this._native.getQuota();
    }
}

// ────────────────────────────────────────────────────
// StatsHelper — wraps jsb.IStats
// ────────────────────────────────────────────────────

export class StatsHelper extends GsHelperBase {
    constructor (native: any, lifecycle: ServicesLifecycle) {
        super(native, lifecycle, 'StatsHelper');
    }

    setStatInt (name: string, value: number): Promise<void> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.setStatInt(name, value, {
                onSuccess: () => resolve(),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    setStatFloat (name: string, value: number): Promise<void> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.setStatFloat(name, value, {
                onSuccess: () => resolve(),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    getStatInt (name: string): { Success: boolean, Value: number } {
        this.assertAlive();
        return this._native.getStatInt(name);
    }

    getStatFloat (name: string): { Success: boolean, Value: number } {
        this.assertAlive();
        return this._native.getStatFloat(name);
    }

    storeStats (): Promise<void> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.storeStats({
                onSuccess: () => resolve(),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    resetAllStats (achievementsToo: boolean): boolean {
        this.assertAlive();
        return this._native.resetAllStats(achievementsToo);
    }
}

// ────────────────────────────────────────────────────
// UtilsHelper — wraps jsb.IUtils
// ────────────────────────────────────────────────────

export class UtilsHelper extends GsHelperBase {
    private _warningListeners = new Set<(severity: number, message: string) => void>();
    private _nativeListenerBound = false;

    constructor (native: any, lifecycle: ServicesLifecycle) {
        super(native, lifecycle, 'UtilsHelper');
    }

    onWarningMessage (callback: (severity: number, message: string) => void): () => void {
        this.assertAlive();
        this._warningListeners.add(callback);

        if (!this._nativeListenerBound) {
            this._nativeListenerBound = true;
            this._native.setWarningMessageHook((sev: number, msg: string) => {
                for (const cb of this._warningListeners) {
                    cb(sev, msg);
                }
            });
        }

        return () => {
            this._warningListeners.delete(callback);
            if (this._warningListeners.size === 0) {
                this._nativeListenerBound = false;
                this._native.setWarningMessageHook(null);
            }
        };
    }

    /**
     * @internal Called by GsServicesHelper during shutdown
     */
    _unbindAllEvents (): void {
        this._warningListeners.clear();
        this._nativeListenerBound = false;
        this._native.setWarningMessageHook(null);
    }
}

// ────────────────────────────────────────────────────
// FriendsHelper — wraps jsb.IFriends
// ────────────────────────────────────────────────────

export interface AvatarImage {
    width: number;
    height: number;
    data: ArrayBuffer;
}

export interface FriendInfo {
    userId: string;
    personaName: string;
    nickname: string;
    personaState: number;
}

export interface FriendsGroupInfo {
    groupId: number;
    groupName: string;
    members: string[];
}

export class FriendsHelper extends GsHelperBase {
    private _joinListeners = new Set<(friendId: string, connectString: string) => void>();
    private _nativeListenerBound = false;

    constructor (native: any, lifecycle: ServicesLifecycle) {
        super(native, lifecycle, 'FriendsHelper');
    }

    getPersonaName (): string {
        this.assertAlive();
        return this._native.getPersonaName();
    }

    getFriends (friendFlags: number): FriendInfo[] {
        this.assertAlive();
        const result = this._native.getFriends(friendFlags);
        return result ? result.Friends : [];
    }

    requestAvatar (userId: string, size: number): Promise<AvatarImage> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.requestAvatar(userId, size, {
                onSuccess: (img: AvatarImage) => resolve(img),
                onFailure: (err: string) => reject(new Error(err)),
            });
        });
    }

    getFriendsGroups (): FriendsGroupInfo[] {
        this.assertAlive();
        const result = this._native.getFriendsGroups();
        return result ? result.Groups : [];
    }

    setRichPresence (key: string, value: string): boolean {
        this.assertAlive();
        return this._native.setRichPresence(key, value);
    }

    clearRichPresence (): void {
        this.assertAlive();
        this._native.clearRichPresence();
    }

    getFriendRichPresence (userId: string, key: string): string {
        this.assertAlive();
        return this._native.getFriendRichPresence(userId, key);
    }

    activateGameOverlay (dialog: number): void {
        this.assertAlive();
        this._native.activateGameOverlay(dialog);
    }

    activateGameOverlayToWebPage (url: string): void {
        this.assertAlive();
        this._native.activateGameOverlayToWebPage(url);
    }

    onGameRichPresenceJoinRequested (callback: (friendId: string, connectString: string) => void): () => void {
        this.assertAlive();
        this._joinListeners.add(callback);

        if (!this._nativeListenerBound) {
            this._nativeListenerBound = true;
            this._native.setOnGameRichPresenceJoinRequested((fid: string, connectStr: string) => {
                for (const cb of this._joinListeners) {
                    cb(fid, connectStr);
                }
            });
        }

        return () => {
            this._joinListeners.delete(callback);
            if (this._joinListeners.size === 0) {
                this._nativeListenerBound = false;
                this._native.setOnGameRichPresenceJoinRequested(null);
            }
        };
    }

    /**
     * @internal Called by GsServicesHelper during shutdown
     */
    _unbindAllEvents (): void {
        this._joinListeners.clear();
        this._nativeListenerBound = false;
        this._native.setOnGameRichPresenceJoinRequested(null);
    }
}

// ────────────────────────────────────────────────────
// GsServicesHelper — wraps jsb.IGsServices
// ────────────────────────────────────────────────────

export class GsServicesHelper {
    private _native: any;
    private _lifecycle = new ServicesLifecycle();
    private _achievements: AchievementsHelper | null = null;
    private _friends: FriendsHelper | null = null;
    private _remoteStorage: RemoteStorageHelper | null = null;
    private _stats: StatsHelper | null = null;
    private _utils: UtilsHelper | null = null;
    private _initialized = false;

    constructor (native: any) {
        this._native = native;
    }

    private assertAlive (): void {
        this._lifecycle.assertAlive('GsServicesHelper');
    }

    restartAppIfNecessary (appId: number | string): boolean {
        this.assertAlive();
        return this._native.restartAppIfNecessary(appId);
    }

    init (): boolean {
        this.assertAlive();
        if (this._initialized) {
            console.warn('[gs] GsServicesHelper is already initialized. Skipping duplicate init().');
            return true;
        }
        const result = this._native.init();
        if (result) {
            this._initialized = true;
        }
        return result;
    }

    destroy (): void {
        // Flip the shared lifecycle first so every handed-out helper (even copies
        // the user kept) starts throwing immediately.
        this._lifecycle.markDestroyed();
        
        // Proactively unregister any active callbacks from the native layer
        // before destroying the native object to prevent dangling callbacks
        // firing into destroyed JS references.
        if (this._achievements) {
            this._achievements._unbindAllEvents();
        }
        if (this._utils) {
            this._utils._unbindAllEvents();
        }
        if (this._friends) {
            this._friends._unbindAllEvents();
        }

        this._native.destroy();
        this._achievements = null;
        this._friends = null;
        this._remoteStorage = null;
        this._stats = null;
        this._utils = null;
        this._initialized = false;
        
        // Remove from cache to allow recreation later
        for (const key in _servicesCache) {
            if (_servicesCache[key] === this) {
                delete _servicesCache[key];
                break;
            }
        }
    }

    getServicesProvider (): number {
        this.assertAlive();
        return this._native.getServicesProvider();
    }

    achievements (): AchievementsHelper {
        this.assertAlive();
        if (!this._achievements) {
            const native = this._native.getAchievementsInterface();
            if (native) {
                this._achievements = new AchievementsHelper(native, this._lifecycle);
            } else {
                throw new Error('[gs] Achievements interface is unavailable. Did you forget to call init() or did init() fail?');
            }
        }
        return this._achievements;
    }

    friends (): FriendsHelper {
        this.assertAlive();
        if (!this._friends) {
            const native = this._native.getFriendsInterface();
            if (native) {
                this._friends = new FriendsHelper(native, this._lifecycle);
            } else {
                throw new Error('[gs] Friends interface is unavailable. Did you forget to call init() or did init() fail?');
            }
        }
        return this._friends;
    }

    remoteStorage (): RemoteStorageHelper {
        this.assertAlive();
        if (!this._remoteStorage) {
            const native = this._native.getRemoteStorageInterface();
            if (native) {
                this._remoteStorage = new RemoteStorageHelper(native, this._lifecycle);
            } else {
                throw new Error('[gs] RemoteStorage interface is unavailable. Did you forget to call init() or did init() fail?');
            }
        }
        return this._remoteStorage;
    }

    stats (): StatsHelper {
        this.assertAlive();
        if (!this._stats) {
            const native = this._native.getStatsInterface();
            if (native) {
                this._stats = new StatsHelper(native, this._lifecycle);
            } else {
                throw new Error('[gs] Stats interface is unavailable. Did you forget to call init() or did init() fail?');
            }
        }
        return this._stats;
    }

    utils (): UtilsHelper {
        this.assertAlive();
        if (!this._utils) {
            const native = this._native.getUtilsInterface();
            if (native) {
                this._utils = new UtilsHelper(native, this._lifecycle);
            } else {
                throw new Error('[gs] Utils interface is unavailable. Did you forget to call init() or did init() fail?');
            }
        }
        return this._utils;
    }
}

// ────────────────────────────────────────────────────
// Factory — wraps jsb.getServices()
// ────────────────────────────────────────────────────

const _servicesCache: Record<string, GsServicesHelper> = {};

export function createServices (
    servicesType: number = jsb.GsServicesType.Steam,
    instanceName: string = '',
    instanceConfigName: string = '',
): GsServicesHelper | null {
    // Generate a unique cache key based on the parameters
    const cacheKey = `${servicesType}_${instanceName}_${instanceConfigName}`;

    if (_servicesCache[cacheKey]) {
        return _servicesCache[cacheKey];
    }

    const native = jsb.IGsServices.getServices(servicesType, instanceName, instanceConfigName);
    if (!native) {
        console.warn(`[gs] getServices failed for type ${servicesType}, no factory registered or creation failed`);
        return null;
    }
    
    const helper = new GsServicesHelper(native);
    _servicesCache[cacheKey] = helper;
    
    return helper;
}

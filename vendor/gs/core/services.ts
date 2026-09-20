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

import { GsError, GsErrorCode, toGsError } from './errors';

declare const jsb: any;

// ────────────────────────────────────────────────────
// Data types matching C++ structs in Achievements.h
// ────────────────────────────────────────────────────

export interface AchievementDefinition {
    id: string;
    displayName: string;
    description: string;
}

export interface AchievementState {
    id: string;
    unlocked: boolean;
    progress: number | null;
    unlockedAt: number | null;
}

// ────────────────────────────────────────────────────
// All helpers consult their original native session; JS has no second lifecycle state.
// ────────────────────────────────────────────────────

class ServicesLifecycle {
    constructor (private readonly native: { isClosed(): boolean; isClosing(): boolean; getState(): number }, readonly provider: number) {}

    isClosed (): boolean {
        return this.native.isClosed() || this.native.isClosing();
    }

    assertReady (who: string): void {
        this.assertAlive(who);
        if (this.native.getState() !== jsb.ServicesState.Ready) {
            throw new GsError(GsErrorCode.NotReady, '[gs] Await init() before using ' + who, this.provider);
        }
    }

    assertAlive (who: string): void {
        if (this.isClosed()) {
            throw new GsError(GsErrorCode.Cancelled, `[gs] ${who} has been destroyed. Fetch a fresh services instance via getServices().`, this.provider);
        }
    }
}

// JS helpers wrap module facades. All checks consult the original native session,
// including when the engine (rather than JS) initiates shutdown.
class GsHelperBase {
    protected readonly _native: any;
    private readonly _lifecycle: ServicesLifecycle;
    private readonly _name: string;

    constructor (native: any, lifecycle: ServicesLifecycle, name: string) {
        this._native = native;
        this._lifecycle = lifecycle;
        this._name = name;
    }

    protected toError (error: unknown): GsError {
        return toGsError(error, this._lifecycle.provider);
    }

    protected assertAlive (): void {
        this._lifecycle.assertAlive(this._name);
    }
}

// ────────────────────────────────────────────────────
// AchievementsHelper — wraps jsb.IAchievements
// ────────────────────────────────────────────────────

export class AchievementsHelper extends GsHelperBase {
    private _achievementListeners = new Set<(state: AchievementState) => void>();
    private _nativeListenerBound = false;

    constructor (native: any, lifecycle: ServicesLifecycle) {
        super(native, lifecycle, 'AchievementsHelper');
    }

    private validateId (id: string): void {
        if (typeof id !== 'string' || !id.length || id.includes('\0')) {
            throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'Invalid achievement ID' });
        }
    }

    async queryDefinitions (): Promise<AchievementDefinition[]> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.queryDefinitions({
                onSuccess: (definitions: AchievementDefinition[]) => resolve(definitions),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async queryStates (): Promise<AchievementState[]> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.queryStates({
                onSuccess: (states: AchievementState[]) => resolve(states),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async unlock (id: string): Promise<void> {
        this.assertAlive();
        this.validateId(id);
        return new Promise((resolve, reject) => {
            this._native.unlock(id, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async clearAchievement (id: string): Promise<void> {
        this.assertAlive();
        this.validateId(id);
        return new Promise((resolve, reject) => {
            this._native.clearAchievement(id, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    onUpdated (callback: (state: AchievementState) => void): () => void {
        this.assertAlive();
        if (typeof callback !== 'function') throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'Expected an event listener' });
        // Each registration owns its unsubscribe token, even for the same callback.
        const subscription = (state: AchievementState): void => callback(state);
        this._achievementListeners.add(subscription);
        if (!this._nativeListenerBound) {
            this._native.setOnUpdated((state: AchievementState) => {
                for (const listener of Array.from(this._achievementListeners)) {
                    if (!this._achievementListeners.has(listener)) continue;
                    try { listener({ ...state }); } catch (error) { console.error(error); }
                }
            });
            this._nativeListenerBound = true;
        }
        return () => {
            this._achievementListeners.delete(subscription);
            if (this._achievementListeners.size === 0 && this._nativeListenerBound) {
                this._nativeListenerBound = false;
                this._native.setOnUpdated(null);
            }
        };
    }

    /** @internal */
    _unbindAllEvents (): void {
        this._achievementListeners.clear();
        this._nativeListenerBound = false;
        this._native.setOnUpdated(null);
    }
}

// ────────────────────────────────────────────────────
// Data types matching C++ structs in RemoteStorage.h
// ────────────────────────────────────────────────────

export interface FileInfo { name: string; size: number; }
export interface QuotaInfo { totalBytes: number; availableBytes: number; }

export class RemoteStorageHelper extends GsHelperBase {
    constructor (native: any, lifecycle: ServicesLifecycle) { super(native, lifecycle, 'RemoteStorageHelper'); }

    private validateName (name: string): void {
        if (typeof name !== 'string' || !name.length || name.includes('\0')) {
            throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'File name must be nonempty and contain no NUL' });
        }
    }

    async writeFile (name: string, data: Uint8Array): Promise<void> {
        this.assertAlive();
        this.validateName(name);
        if (!(data instanceof Uint8Array)) throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'File data must be a Uint8Array' });
        return new Promise((resolve, reject) => {
            this._native.writeFile(name, data, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async readFile (name: string): Promise<Uint8Array> {
        this.assertAlive();
        this.validateName(name);
        return new Promise((resolve, reject) => {
            this._native.readFile(name, {
                onSuccess: (value: Uint8Array) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async deleteFile (name: string): Promise<void> {
        this.assertAlive();
        this.validateName(name);
        return new Promise((resolve, reject) => {
            this._native.deleteFile(name, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async getFileInfo (name: string): Promise<FileInfo | null> {
        this.assertAlive();
        this.validateName(name);
        return new Promise((resolve, reject) => {
            this._native.getFileInfo(name, {
                onSuccess: (value: FileInfo | null) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async listFiles (): Promise<FileInfo[]> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.listFiles({
                onSuccess: (value: FileInfo[]) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async getQuota (): Promise<QuotaInfo> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.getQuota({
                onSuccess: (value: QuotaInfo) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async writeText (name: string, text: string): Promise<void> {
        this.assertAlive();
        this.validateName(name);
        if (typeof text !== 'string') throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'Text must be a string' });
        let data: Uint8Array;
        try { data = new TextEncoder().encode(text); }
        catch (error) { throw this.toError(error); }
        return this.writeFile(name, data);
    }

    async readText (name: string): Promise<string> {
        const data = await this.readFile(name);
        try { return new TextDecoder('utf-8').decode(data); }
        catch (error) { throw this.toError(error); }
    }
}

export interface ResetStatsOptions { includeAchievements?: boolean; }

export class StatsHelper extends GsHelperBase {
    constructor (native: any, lifecycle: ServicesLifecycle) {
        super(native, lifecycle, 'StatsHelper');
    }

    private validateName (name: string): void {
        if (typeof name !== 'string' || !name.length || name.includes('\0')) {
            throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'Invalid stat name' });
        }
    }

    private validateValue (value: number, integer: boolean): void {
        if (integer ? !Number.isSafeInteger(value) : !Number.isFinite(value)) {
            throw this.toError({ code: GsErrorCode.InvalidArgument, message: integer ? 'Expected a safe integer' : 'Expected a finite number' });
        }
    }

    async getInt (name: string): Promise<number> {
        this.assertAlive();
        this.validateName(name);
        return new Promise((resolve, reject) => {
            this._native.getInt(name, {
                onSuccess: (value: number) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async getFloat (name: string): Promise<number> {
        this.assertAlive();
        this.validateName(name);
        return new Promise((resolve, reject) => {
            this._native.getFloat(name, {
                onSuccess: (value: number) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async setInt (name: string, value: number): Promise<void> {
        this.assertAlive();
        this.validateName(name);
        this.validateValue(value, true);
        return new Promise((resolve, reject) => {
            this._native.setInt(name, value, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async setFloat (name: string, value: number): Promise<void> {
        this.assertAlive();
        this.validateName(name);
        this.validateValue(value, false);
        return new Promise((resolve, reject) => {
            this._native.setFloat(name, value, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async incrementInt (name: string, delta: number): Promise<void> {
        this.assertAlive();
        this.validateName(name);
        this.validateValue(delta, true);
        return new Promise((resolve, reject) => {
            this._native.incrementInt(name, delta, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async incrementFloat (name: string, delta: number): Promise<void> {
        this.assertAlive();
        this.validateName(name);
        this.validateValue(delta, false);
        return new Promise((resolve, reject) => {
            this._native.incrementFloat(name, delta, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async flush (): Promise<void> {
        this.assertAlive();

        return new Promise((resolve, reject) => {
            this._native.flush({
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async resetAll (options: ResetStatsOptions = {}): Promise<void> {
        this.assertAlive();
        if (!options || typeof options !== 'object'
            || (options.includeAchievements !== undefined && typeof options.includeAchievements !== 'boolean')) {
            throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'Invalid reset options' });
        }
        return new Promise((resolve, reject) => {
            this._native.resetAll(options.includeAchievements ?? false, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }
}

// ────────────────────────────────────────────────────
// UtilsHelper — wraps jsb.IUtils
// ────────────────────────────────────────────────────

export interface DiagnosticMessage { level: number; message: string; }

export class UtilsHelper extends GsHelperBase {
    private _listeners = new Set<(message: DiagnosticMessage) => void>();
    private _nativeListenerBound = false;

    constructor (native: any, lifecycle: ServicesLifecycle) {
        super(native, lifecycle, 'UtilsHelper');
    }

    onDiagnostic (callback: (message: DiagnosticMessage) => void): () => void {
        this.assertAlive();
        if (typeof callback !== 'function') throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'Expected an event listener' });
        // Each registration owns its unsubscribe token, even for the same callback.
        const subscription = (message: DiagnosticMessage): void => callback(message);
        this._listeners.add(subscription);
        if (!this._nativeListenerBound) {
            this._native.setOnDiagnostic((message: DiagnosticMessage) => {
                for (const listener of Array.from(this._listeners)) {
                    if (!this._listeners.has(listener)) continue;
                    try { listener({ ...message }); } catch (error) { console.error(error); }
                }
            });
            this._nativeListenerBound = true;
        }
        return () => {
            this._listeners.delete(subscription);
            if (this._listeners.size === 0 && this._nativeListenerBound) {
                this._nativeListenerBound = false;
                this._native.setOnDiagnostic(null);
            }
        };
    }

    /** @internal */
    _unbindAllEvents (): void {
        this._listeners.clear();
        this._nativeListenerBound = false;
        this._native.setOnDiagnostic(null);
    }
}

// ────────────────────────────────────────────────────
// Account and friends data
// ────────────────────────────────────────────────────

export interface UserProfile { userId: string; displayName: string; }
export interface FriendInfo extends UserProfile { nickname: string | null; presence: number; }
export interface AvatarImage { width: number; height: number; data: Uint8Array; }
export interface FriendGroup { id: string; displayName: string; memberIds: string[]; }
export interface JoinRequest { userId: string; connectionString: string; }

export class AccountHelper extends GsHelperBase {
    constructor (native: any, lifecycle: ServicesLifecycle) {
        super(native, lifecycle, 'AccountHelper');
    }

    async getUser (): Promise<UserProfile | null> {
        this.assertAlive();

        return new Promise((resolve, reject) => {
            this._native.getUser({
                onSuccess: (value: UserProfile | null) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async login (): Promise<UserProfile> {
        this.assertAlive();

        return new Promise((resolve, reject) => {
            this._native.login({
                onSuccess: (value: UserProfile) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }
}

// FriendsHelper — wraps jsb.IFriends
export class FriendsHelper extends GsHelperBase {
    private _joinListeners = new Set<(request: JoinRequest) => void>();
    private _nativeListenerBound = false;

    constructor (native: any, lifecycle: ServicesLifecycle) {
        super(native, lifecycle, 'FriendsHelper');
    }

    private validateText (value: string, label: string, allowEmpty = false): void {
        if (typeof value !== 'string' || (!allowEmpty && !value.length) || value.includes('\0')) {
            throw this.toError({ code: GsErrorCode.InvalidArgument, message: label + ' is invalid' });
        }
    }

    async getFriends (): Promise<FriendInfo[]> {
        this.assertAlive();

        return new Promise((resolve, reject) => {
            this._native.getFriends({
                onSuccess: (value: FriendInfo[]) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async getAvatar (userId: string, size: number = 1): Promise<AvatarImage | null> {
        this.assertAlive();
        this.validateText(userId, 'User ID');
        if (!Number.isInteger(size) || size < 0 || size > 2) throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'Invalid avatar size' });
        return new Promise((resolve, reject) => {
            this._native.getAvatar(userId, size, {
                onSuccess: (value: AvatarImage | null) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async getGroups (): Promise<FriendGroup[]> {
        this.assertAlive();

        return new Promise((resolve, reject) => {
            this._native.getGroups({
                onSuccess: (value: FriendGroup[]) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async setRichPresence (key: string, value: string): Promise<void> {
        this.assertAlive();
        this.validateText(key, 'Presence key');
        this.validateText(value, 'Presence value', true);
        return new Promise((resolve, reject) => {
            this._native.setRichPresence(key, value, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async clearRichPresence (): Promise<void> {
        this.assertAlive();

        return new Promise((resolve, reject) => {
            this._native.clearRichPresence({
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async getRichPresence (userId: string, key: string): Promise<string | null> {
        this.assertAlive();
        this.validateText(userId, 'User ID');
        this.validateText(key, 'Presence key');
        return new Promise((resolve, reject) => {
            this._native.getRichPresence(userId, key, {
                onSuccess: (value: string | null) => resolve(value),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async openOverlay (dialog: number): Promise<void> {
        this.assertAlive();
        if (!Number.isInteger(dialog) || dialog < 0 || dialog > 6) throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'Invalid overlay page' });
        return new Promise((resolve, reject) => {
            this._native.openOverlay(dialog, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    async openWebPage (url: string): Promise<void> {
        this.assertAlive();
        this.validateText(url, 'URL');
        if (!/^https?:\/\//.test(url)) throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'Expected an HTTP or HTTPS URL' });
        return new Promise((resolve, reject) => {
            this._native.openWebPage(url, {
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(this.toError(error)),
            });
        });
    }

    onJoinRequested (callback: (request: JoinRequest) => void): () => void {
        this.assertAlive();
        if (typeof callback !== 'function') throw this.toError({ code: GsErrorCode.InvalidArgument, message: 'Expected an event listener' });
        // Each registration owns its unsubscribe token, even for the same callback.
        const subscription = (request: JoinRequest): void => callback(request);
        this._joinListeners.add(subscription);
        if (!this._nativeListenerBound) {
            this._native.setOnJoinRequested((request: JoinRequest) => {
                for (const listener of Array.from(this._joinListeners)) {
                    if (!this._joinListeners.has(listener)) continue;
                    try { listener({ ...request }); } catch (error) { console.error(error); }
                }
            });
            this._nativeListenerBound = true;
        }
        return () => {
            this._joinListeners.delete(subscription);
            if (this._joinListeners.size === 0 && this._nativeListenerBound) {
                this._nativeListenerBound = false;
                this._native.setOnJoinRequested(null);
            }
        };
    }

    /** @internal */
    _unbindAllEvents (): void {
        this._joinListeners.clear();
        this._nativeListenerBound = false;
        this._native.setOnJoinRequested(null);
    }
}

// ────────────────────────────────────────────────────
// GsServicesHelper — wraps jsb.IGsServices
// ────────────────────────────────────────────────────

export class GsServicesHelper {
    private _native: any;
    private _lifecycle: ServicesLifecycle;
    private _achievements: AchievementsHelper | null = null;
    private _friends: FriendsHelper | null = null;
    private _remoteStorage: RemoteStorageHelper | null = null;
    private _stats: StatsHelper | null = null;
    private _utils: UtilsHelper | null = null;
    private _account: AccountHelper | null = null;

    constructor (native: any) {
        this._native = native;
        this._lifecycle = new ServicesLifecycle(native, native.getServicesProvider());
    }

    private assertAlive (): void {
        this._lifecycle.assertAlive('GsServicesHelper');
    }

    async restartAppIfNecessary (appId: number | string): Promise<boolean> {
        this.assertAlive();
        const valid = typeof appId === 'number'
            ? Number.isInteger(appId) && appId >= 0 && appId <= 0xFFFFFFFF
            : typeof appId === 'string' && appId.length > 0 && !appId.includes('\0');
        if (!valid) throw new GsError(GsErrorCode.InvalidArgument, 'Invalid app ID', this._lifecycle.provider);
        return new Promise((resolve, reject) => {
            this._native.restartAppIfNecessary(appId, {
                onSuccess: (required: boolean) => resolve(required),
                onFailure: (error: unknown) => reject(toGsError(error, this._lifecycle.provider)),
            });
        });
    }

    async init (): Promise<void> {
        this.assertAlive();
        return new Promise((resolve, reject) => {
            this._native.init({
                onSuccess: () => resolve(),
                onFailure: (error: unknown) => reject(toGsError(error, this._lifecycle.provider)),
            });
        });
    }

    getState (): number { return this._native.getState(); }

    hasModule (module: number): boolean {
        this._lifecycle.assertReady('hasModule');
        if (!Number.isInteger(module) || module < 0 || module > jsb.ServicesModule.Account) {
            throw new GsError(GsErrorCode.InvalidArgument, 'Invalid module', this._lifecycle.provider);
        }
        return this._native.hasModule(module);
    }

    /** @internal Used to discard a cached wrapper closed by native lifecycle cleanup. */
    _isClosed (): boolean {
        return this._lifecycle.isClosed();
    }

    destroy (): void {
        // Native session state is authoritative, including closes initiated by the engine.
        // It rejects new calls immediately and defers backend deletion during dispatch.
        this._native.destroy();
        if (this._achievements) {
            this._achievements._unbindAllEvents();
        }
        if (this._utils) {
            this._utils._unbindAllEvents();
        }
        if (this._friends) {
            this._friends._unbindAllEvents();
        }

        this._achievements = null;
        this._friends = null;
        this._remoteStorage = null;
        this._stats = null;
        this._utils = null;
        this._account = null;
        
        // Remove from cache to allow recreation later
        for (const [provider, services] of _servicesCache) {
            if (services === this) {
                _servicesCache.delete(provider);
                break;
            }
        }
    }

    getServicesProvider (): number { return this._lifecycle.provider; }

    achievements (): AchievementsHelper {
        this._lifecycle.assertReady('achievements');
        if (!this._achievements) {
            const native = this._native.getAchievementsInterface();
            if (native) {
                this._achievements = new AchievementsHelper(native, this._lifecycle);
            } else {
                throw new GsError(GsErrorCode.NotSupported, '[gs] Achievements is not supported by this session', this._lifecycle.provider);
            }
        }
        return this._achievements;
    }

    friends (): FriendsHelper {
        this._lifecycle.assertReady('friends');
        if (!this._friends) {
            const native = this._native.getFriendsInterface();
            if (native) {
                this._friends = new FriendsHelper(native, this._lifecycle);
            } else {
                throw new GsError(GsErrorCode.NotSupported, '[gs] Friends is not supported by this session', this._lifecycle.provider);
            }
        }
        return this._friends;
    }

    remoteStorage (): RemoteStorageHelper {
        this._lifecycle.assertReady('remoteStorage');
        if (!this._remoteStorage) {
            const native = this._native.getRemoteStorageInterface();
            if (native) {
                this._remoteStorage = new RemoteStorageHelper(native, this._lifecycle);
            } else {
                throw new GsError(GsErrorCode.NotSupported, '[gs] RemoteStorage is not supported by this session', this._lifecycle.provider);
            }
        }
        return this._remoteStorage;
    }

    stats (): StatsHelper {
        this._lifecycle.assertReady('stats');
        if (!this._stats) {
            const native = this._native.getStatsInterface();
            if (native) {
                this._stats = new StatsHelper(native, this._lifecycle);
            } else {
                throw new GsError(GsErrorCode.NotSupported, '[gs] Stats is not supported by this session', this._lifecycle.provider);
            }
        }
        return this._stats;
    }

    account (): AccountHelper {
        this._lifecycle.assertReady('account');
        if (!this._account) {
            const native = this._native.getAccountInterface();
            if (!native) throw new GsError(GsErrorCode.NotSupported, '[gs] Account is not supported by this session', this._lifecycle.provider);
            this._account = new AccountHelper(native, this._lifecycle);
        }
        return this._account;
    }

    utils (): UtilsHelper {
        this._lifecycle.assertReady('utils');
        if (!this._utils) {
            const native = this._native.getUtilsInterface();
            if (native) {
                this._utils = new UtilsHelper(native, this._lifecycle);
            } else {
                throw new GsError(GsErrorCode.NotSupported, '[gs] Utils is not supported by this session', this._lifecycle.provider);
            }
        }
        return this._utils;
    }
}

// ────────────────────────────────────────────────────
// Factory — wraps jsb.getServices()
// ────────────────────────────────────────────────────

const _servicesCache = new Map<number, GsServicesHelper>();

export function createServices (
    servicesType: number = jsb.GsServicesType.Steam,
): GsServicesHelper | null {
    if (!Number.isInteger(servicesType) || servicesType < 0 || servicesType > 255) {
        throw new GsError(GsErrorCode.InvalidArgument, 'Invalid services provider', servicesType);
    }
    const cached = _servicesCache.get(servicesType);
    if (cached && !cached._isClosed()) {
        return cached;
    }
    _servicesCache.delete(servicesType);

    const native = jsb.IGsServices.getServices(servicesType);
    if (!native) {
        console.warn(`[gs] getServices failed for type ${servicesType}, no factory registered or creation failed`);
        return null;
    }
    
    const helper = new GsServicesHelper(native);
    _servicesCache.set(servicesType, helper);
    
    return helper;
}

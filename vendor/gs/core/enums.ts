/* eslint-disable @typescript-eslint/no-namespace */
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
import { JSB } from 'internal:constants';

declare const jsb: any;

// SWIG does not register the gs enums into the jsb namespace, so their values
// are defined here and must match the C++ enums they mirror.
if (JSB && jsb.IGsServices) {
    jsb.GsServicesType = {
        Null: 0,
        Steam: 1,
        Epic: 2, // Not implemented yet — no native module/factory registered.
    };
    // cc::Gs::PersonaState in Framework/Friends.h
    jsb.PersonaState = {
        Offline: 0,
        Online: 1,
        Busy: 2,
        Away: 3,
        Snooze: 4,
        LookingToTrade: 5,
        LookingToPlay: 6,
        Invisible: 7,
    };
    // cc::Gs::AvatarSize in Framework/Friends.h
    jsb.AvatarSize = {
        Small: 0,
        Medium: 1,
        Large: 2,
    };
    // cc::Gs::FriendFlags in Framework/Friends.h
    jsb.FriendFlags = {
        None: 0,
        Immediate: 1,
        Blocked: 2,
        FriendshipRequested: 4,
        RequestingFriendship: 8,
        ClanMember: 16,
        OnGameServer: 32,
        All: 0xFFFFFFFF,
    };
    // cc::Gs::OverlayDialog in Framework/Friends.h
    jsb.OverlayDialog = {
        Friends: 0,
        Community: 1,
        Players: 2,
        Settings: 3,
        OfficialGameGroup: 4,
        Stats: 5,
        Achievements: 6,
    };
}

export {};

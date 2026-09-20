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

#pragma once

#include <cmath>
#include <limits>

#include "bindings/jswrapper/SeApi.h"
#include "bindings/manual/jsb_conversions.h"
#include "Achievements.h"
#include "Friends.h"
#include "Account.h"
#include "RemoteStorage.h"
#include "Stats.h"
#include "Utils.h"
#include "commons/GsCallback.h"
#include "commons/GsTypes.h"
#include "../common/ScopedListener.h"
#include "../common/JsUtils.h"

namespace cc::Gs {

inline bool nativevalue_to_se(const DiagnosticMessage& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("level", se::Value(static_cast<int>(from.level)));
    obj->setProperty("message", se::Value(from.message));
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const GsError& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("code", se::Value(static_cast<int>(from.code)));
    obj->setProperty("message", se::Value(from.message));
    se::Value platformCode;
    if (from.platformCode.empty()) platformCode.setNull();
    else platformCode.setString(from.platformCode);
    obj->setProperty("platformCode", platformCode);
    to.setObject(obj);
    return true;
}
inline bool nativevalue_to_se(const AchievementDefinition& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("id", se::Value(from.id));
    obj->setProperty("displayName", se::Value(from.displayName));
    obj->setProperty("description", se::Value(from.description));
    to.setObject(obj);
    return true;
}
inline bool nativevalue_to_se(const AchievementState& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("id", se::Value(from.id));
    obj->setProperty("unlocked", se::Value(from.unlocked));
    se::Value progress, time;
    if (from.progress) progress.setNumber(*from.progress);
    else progress.setNull();
    if (from.unlockedAt) time.setNumber(static_cast<double>(*from.unlockedAt));
    else time.setNull();
    obj->setProperty("progress", progress);
    obj->setProperty("unlockedAt", time);
    to.setObject(obj);
    return true;
}
inline bool nativevalue_to_se(const std::vector<AchievementDefinition>& from, se::Value& to, se::Object* ctx) {
    se::HandleObject array(se::Object::createArrayObject(from.size()));
    for (uint32_t i = 0; i < from.size(); ++i) {
        se::Value value;
        nativevalue_to_se(from[i], value, ctx);
        array->setArrayElement(i, value);
    }
    to.setObject(array);
    return true;
}
inline bool nativevalue_to_se(const std::vector<AchievementState>& from, se::Value& to, se::Object* ctx) {
    se::HandleObject array(se::Object::createArrayObject(from.size()));
    for (uint32_t i = 0; i < from.size(); ++i) {
        se::Value value;
        nativevalue_to_se(from[i], value, ctx);
        array->setArrayElement(i, value);
    }
    to.setObject(array);
    return true;
}

inline bool sevalue_to_native(const se::Value& from, cc::Gs::AppId* to, se::Object*) {
    if (from.isNumber()) {
        const double value = from.toDouble();
        // Only validate lossless conversion here; platform-specific ID rules belong to the backend.
        if (!std::isfinite(value) || std::floor(value) != value
            || value < 0 || value > (std::numeric_limits<uint32_t>::max)()) return false;
        *to = static_cast<uint32_t>(value);
    } else if (from.isString()) {
        *to = from.toString();
    } else {
        return false;
    }
    return true;
}

// Only this adapter layer owns JS handles; callbacks used by backends are native callables.
template <typename... Args>
inline bool sevalue_to_native(const se::Value& from, cc::Gs::AsyncCallback<Args...>* to, se::Object*) {
    if (!from.isObject()) { *to = {}; return true; }
    scopedListener listener(from.toObject());
    *to = AsyncCallback<Args...>(
        [listener](Args... args) { callJSfunc(listener.get(), "onSuccess", args...); },
        [listener](const GsError& error) { callJSfunc(listener.get(), "onFailure", error); });
    return true;
}

template <typename... Args>
inline bool sevalue_to_native(const se::Value& from, cc::Gs::EventDelegate<Args...>* to, se::Object*) {
    if (!from.isObject()) { *to = {}; return true; }
    scopedListener listener(from.toObject());
    *to = EventDelegate<Args...>([listener](const Args&... args) { invokeJSfunc(listener.get(), args...); });
    return true;
}

inline bool nativevalue_to_se(const FileInfo& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("name", se::Value(from.name));
    obj->setProperty("size", se::Value(static_cast<double>(from.size)));
    to.setObject(obj);
    return true;
}
inline bool nativevalue_to_se(const std::optional<FileInfo>& from, se::Value& to, se::Object* ctx) {
    if (!from) { to.setNull(); return true; }
    return nativevalue_to_se(*from, to, ctx);
}
inline bool nativevalue_to_se(const QuotaInfo& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("totalBytes", se::Value(static_cast<double>(from.totalBytes)));
    obj->setProperty("availableBytes", se::Value(static_cast<double>(from.availableBytes)));
    to.setObject(obj);
    return true;
}
inline bool nativevalue_to_se(const std::vector<FileInfo>& from, se::Value& to, se::Object* ctx) {
    se::HandleObject array(se::Object::createArrayObject(from.size()));
    for (uint32_t i = 0; i < from.size(); ++i) {
        se::Value value;
        nativevalue_to_se(from[i], value, ctx);
        array->setArrayElement(i, value);
    }
    to.setObject(array);
    return true;
}
inline bool nativevalue_to_se(const FileData& from, se::Value& to, se::Object*) {
    se::HandleObject array(se::Object::createTypedArray(se::Object::TypedArrayType::UINT8, from.bytes.data(), from.bytes.size()));
    to.setObject(array);
    return true;
}
inline bool sevalue_to_native(const se::Value& from, FileData* to, se::Object*) {
    if (!from.isObject() || !from.toObject()->isTypedArray()
        || from.toObject()->getTypedArrayType() != se::Object::TypedArrayType::UINT8) return false;
    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (!from.toObject()->getTypedArrayData(&bytes, &length)) return false;
    to->bytes.clear();
    if (length) to->bytes.assign(bytes, bytes + length);
    return true;
}

inline bool nativevalue_to_se(const UserProfile& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("userId", se::Value(from.userId));
    obj->setProperty("displayName", se::Value(from.displayName));
    to.setObject(obj);
    return true;
}
inline bool nativevalue_to_se(const std::optional<UserProfile>& from, se::Value& to, se::Object* ctx) {
    if (!from) { to.setNull(); return true; }
    return nativevalue_to_se(*from, to, ctx);
}
inline bool nativevalue_to_se(const FriendInfo& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("userId", se::Value(from.userId));
    obj->setProperty("displayName", se::Value(from.displayName));
    se::Value nickname;
    if (from.nickname) nickname.setString(*from.nickname);
    else nickname.setNull();
    obj->setProperty("nickname", nickname);
    obj->setProperty("presence", se::Value(static_cast<int>(from.presence)));
    to.setObject(obj);
    return true;
}
inline bool nativevalue_to_se(const AvatarImage& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("width", se::Value(from.width));
    obj->setProperty("height", se::Value(from.height));
    se::HandleObject data(se::Object::createTypedArray(se::Object::TypedArrayType::UINT8, from.data.data(), from.data.size()));
    se::Value value;
    value.setObject(data);
    obj->setProperty("data", value);
    to.setObject(obj);
    return true;
}
inline bool nativevalue_to_se(const std::optional<AvatarImage>& from, se::Value& to, se::Object* ctx) {
    if (!from) { to.setNull(); return true; }
    return nativevalue_to_se(*from, to, ctx);
}
inline bool nativevalue_to_se(const FriendGroup& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("id", se::Value(from.id));
    obj->setProperty("displayName", se::Value(from.displayName));
    se::HandleObject array(se::Object::createArrayObject(from.memberIds.size()));
    for (uint32_t i = 0; i < from.memberIds.size(); ++i) array->setArrayElement(i, se::Value(from.memberIds[i]));
    se::Value members;
    members.setObject(array);
    obj->setProperty("memberIds", members);
    to.setObject(obj);
    return true;
}
inline bool nativevalue_to_se(const PresenceValue& from, se::Value& to, se::Object*) {
    if (from.value) to.setString(*from.value);
    else to.setNull();
    return true;
}
inline bool nativevalue_to_se(const JoinRequest& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("userId", se::Value(from.userId));
    obj->setProperty("connectionString", se::Value(from.connectionString));
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const std::vector<FriendInfo>& from, se::Value& to, se::Object* ctx) {
    se::HandleObject array(se::Object::createArrayObject(from.size()));
    for (uint32_t i = 0; i < from.size(); ++i) {
        se::Value value;
        nativevalue_to_se(from[i], value, ctx);
        array->setArrayElement(i, value);
    }
    to.setObject(array);
    return true;
}

inline bool nativevalue_to_se(const std::vector<FriendGroup>& from, se::Value& to, se::Object* ctx) {
    se::HandleObject array(se::Object::createArrayObject(from.size()));
    for (uint32_t i = 0; i < from.size(); ++i) {
        se::Value value;
        nativevalue_to_se(from[i], value, ctx);
        array->setArrayElement(i, value);
    }
    to.setObject(array);
    return true;
}

} // namespace cc::Gs

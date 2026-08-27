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

#include "bindings/jswrapper/SeApi.h"
#include "bindings/manual/jsb_conversions.h"
#include "Achievements.h"
#include "Friends.h"
#include "RemoteStorage.h"
#include "Stats.h"
#include "commons/GsCallback.h"
#include "commons/GsTypes.h"

namespace cc::Gs {

inline bool nativevalue_to_se(const AchievementDefinition& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("AchievementId", se::Value(from.AchievementId));
    obj->setProperty("DisplayName", se::Value(from.DisplayName));
    obj->setProperty("Description", se::Value(from.Description));
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const AchievementState& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("AchievementId", se::Value(from.AchievementId));
    obj->setProperty("Progress", se::Value(from.Progress));
    obj->setProperty("UnlockTimeSec", se::Value(from.UnlockTimeSec));
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const AchievementIdsResult& from, se::Value& to, se::Object*) {
    se::HandleObject arr(se::Object::createArrayObject(from.AchievementIds.size()));
    for (uint32_t i = 0; i < from.AchievementIds.size(); i++) {
        arr->setArrayElement(i, se::Value(from.AchievementIds[i]));
    }
    se::HandleObject obj(se::Object::createPlainObject());
    se::Value arrVal;
    arrVal.setObject(arr);
    obj->setProperty("AchievementIds", arrVal);
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const AchievementDefinitionResult& from, se::Value& to, se::Object* ctx) {
    se::HandleObject obj(se::Object::createPlainObject());
    se::Value defVal;
    nativevalue_to_se(from.Definition, defVal, ctx);
    obj->setProperty("Definition", defVal);
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const AchievementStateResult& from, se::Value& to, se::Object* ctx) {
    se::HandleObject obj(se::Object::createPlainObject());
    se::Value stateVal;
    nativevalue_to_se(from.State, stateVal, ctx);
    obj->setProperty("State", stateVal);
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const StatIntResult& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("Success", se::Value(from.Success));
    obj->setProperty("Value", se::Value(from.Value));
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const StatFloatResult& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("Success", se::Value(from.Success));
    obj->setProperty("Value", se::Value(from.Value));
    to.setObject(obj);
    return true;
}

inline bool sevalue_to_native(const se::Value& from, cc::Gs::AppId* to, se::Object*) {
    if (from.isNumber()) {
        *to = from.toUint32();
    } else if (from.isString()) {
        *to = from.toString();
    } else {
        CC_ASSERT(false);
        return false;
    }
    return true;
}

inline bool sevalue_to_native(const se::Value& from, cc::Gs::AsyncCallbackBase* to, se::Object*) {
    to->bind(from.isObject() ? from.toObject() : nullptr);
    return true;
}

template <typename... Args>
inline bool sevalue_to_native(const se::Value& from, cc::Gs::AsyncCallback<Args...>* to, se::Object*) {
    to->bind(from.isObject() ? from.toObject() : nullptr);
    return true;
}


inline bool sevalue_to_native(const se::Value& from, cc::Gs::EventDelegateBase* to, se::Object*) {
    to->bind(from.isObject() ? from.toObject() : nullptr);
    return true;
}

template<typename... Args>
inline bool sevalue_to_native(const se::Value& from, cc::Gs::EventDelegate<Args...>* to, se::Object*) {
    to->bind(from.isObject() ? from.toObject() : nullptr);
    return true;
}

inline bool nativevalue_to_se(const FileInfo& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("FileName", se::Value(from.FileName));
    obj->setProperty("FileSize", se::Value(from.FileSize));
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const QuotaInfo& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("TotalBytes", se::Value(static_cast<double>(from.TotalBytes)));
    obj->setProperty("AvailableBytes", se::Value(static_cast<double>(from.AvailableBytes)));
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const FileList& from, se::Value& to, se::Object* ctx) {
    se::HandleObject arr(se::Object::createArrayObject(from.Files.size()));
    for (uint32_t i = 0; i < from.Files.size(); i++) {
        se::Value fileVal;
        nativevalue_to_se(from.Files[i], fileVal, ctx);
        arr->setArrayElement(i, fileVal);
    }
    se::HandleObject obj(se::Object::createPlainObject());
    se::Value arrVal;
    arrVal.setObject(arr);
    obj->setProperty("Files", arrVal);
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const FriendInfo& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("userId", se::Value(from.userId));
    obj->setProperty("personaName", se::Value(from.personaName));
    obj->setProperty("nickname", se::Value(from.nickname));
    obj->setProperty("personaState", se::Value(static_cast<int>(from.personaState)));
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const FriendListResult& from, se::Value& to, se::Object* ctx) {
    se::HandleObject arr(se::Object::createArrayObject(from.Friends.size()));
    for (uint32_t i = 0; i < from.Friends.size(); i++) {
        se::Value val;
        nativevalue_to_se(from.Friends[i], val, ctx);
        arr->setArrayElement(i, val);
    }
    se::HandleObject obj(se::Object::createPlainObject());
    se::Value arrVal;
    arrVal.setObject(arr);
    obj->setProperty("Friends", arrVal);
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const AvatarImage& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("width", se::Value(from.width));
    obj->setProperty("height", se::Value(from.height));
    se::HandleObject buf(se::Object::createArrayBufferObject(from.data.data(), from.data.size()));
    se::Value bufVal;
    bufVal.setObject(buf);
    obj->setProperty("data", bufVal);
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const FriendsGroupInfo& from, se::Value& to, se::Object*) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("groupId", se::Value(static_cast<int>(from.groupId)));
    obj->setProperty("groupName", se::Value(from.groupName));
    se::HandleObject arr(se::Object::createArrayObject(from.members.size()));
    for (uint32_t i = 0; i < from.members.size(); i++) {
        arr->setArrayElement(i, se::Value(from.members[i]));
    }
    se::Value arrVal;
    arrVal.setObject(arr);
    obj->setProperty("members", arrVal);
    to.setObject(obj);
    return true;
}

inline bool nativevalue_to_se(const FriendsGroupListResult& from, se::Value& to, se::Object* ctx) {
    se::HandleObject arr(se::Object::createArrayObject(from.Groups.size()));
    for (uint32_t i = 0; i < from.Groups.size(); i++) {
        se::Value val;
        nativevalue_to_se(from.Groups[i], val, ctx);
        arr->setArrayElement(i, val);
    }
    se::HandleObject obj(se::Object::createPlainObject());
    se::Value arrVal;
    arrVal.setObject(arr);
    obj->setProperty("Groups", arrVal);
    to.setObject(obj);
    return true;
}

} // namespace cc::Gs

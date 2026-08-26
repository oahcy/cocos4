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

#include <string>

#ifndef SWIG
#include "vendor/gs/common/ScopedListener.h"
#include "vendor/gs/common/JsUtils.h"
#endif

namespace cc::Gs {

// ── One-shot async callback (auto-reset after success/failure) ──

class AsyncCallbackBase {
public:
    AsyncCallbackBase() = default;

#ifndef SWIG
    ~AsyncCallbackBase() = default;
    AsyncCallbackBase(const AsyncCallbackBase&) = default;
    AsyncCallbackBase& operator=(const AsyncCallbackBase&) = default;
    AsyncCallbackBase(AsyncCallbackBase&&) noexcept = default;
    AsyncCallbackBase& operator=(AsyncCallbackBase&&) noexcept = default;

    void bind(se::Object* obj) { _listener.reset(obj); }

    void success() {
        if (_listener) {
            callJSfunc(_listener.get(), "onSuccess");
            _listener.reset();
        }
    }

    void failure(const std::string& message) {
        if (_listener) {
            callJSfunc(_listener.get(), "onFailure", message);
            _listener.reset();
        }
    }

    explicit operator bool() const { return static_cast<bool>(_listener); }
    void reset() { _listener.reset(); }

protected:
    scopedListener _listener;
#endif
};

using OnComplete = AsyncCallbackBase;

// ── Persistent event delegate (survives multiple invocations) ──

class EventDelegateBase {
public:
    EventDelegateBase() = default;

#ifndef SWIG
    ~EventDelegateBase() = default;
    EventDelegateBase(const EventDelegateBase&) = default;
    EventDelegateBase& operator=(const EventDelegateBase&) = default;
    EventDelegateBase(EventDelegateBase&&) noexcept = default;
    EventDelegateBase& operator=(EventDelegateBase&&) noexcept = default;

    void bind(se::Object* obj) { _listener.reset(obj); }

    explicit operator bool() const { return static_cast<bool>(_listener); }
    void reset() { _listener.reset(); }

protected:
    scopedListener _listener;
#endif
};

#ifndef SWIG
template<typename... Args>
class EventDelegate : public EventDelegateBase {
public:
    void invoke(const Args&... args) {
        if (_listener) {
            invokeJSfunc(_listener.get(), args...);
        }
    }
};

template<typename... Args>
class AsyncCallback : public AsyncCallbackBase {
public:
    void success(const Args&... args) {
        if (_listener) {
            callJSfunc(_listener.get(), "onSuccess", args...);
            _listener.reset();
        }
    }
};
#endif

#ifdef SWIG
using OnAchievementStateUpdated = EventDelegateBase;
using OnReadFile = AsyncCallbackBase;
using OnWarningMessage = EventDelegateBase;
using OnGameRichPresenceJoinRequested = EventDelegateBase;
#else
using OnAchievementStateUpdated = EventDelegate<std::string, float, uint32_t>;
using OnReadFile = AsyncCallback<std::string>;
using OnWarningMessage = EventDelegate<int, std::string>;
using OnGameRichPresenceJoinRequested = EventDelegate<std::string, std::string>;
#endif

} // namespace cc::Gs

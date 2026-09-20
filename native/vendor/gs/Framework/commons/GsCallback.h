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
#include "GsError.h"
#ifndef SWIG
#include <functional>
#include <memory>
#include <utility>
#endif

namespace cc::Gs {

#ifdef SWIG
class AsyncCallbackBase {};
class EventDelegateBase {};
using OnComplete = AsyncCallbackBase;
#else
// Shared by callbacks and the session; contains no back-reference to the session.
struct SessionGate { bool active = false; };

class PendingCallback {
public:
    virtual ~PendingCallback() = default;
    virtual void cancel(const GsError& error) = 0;
    bool completed = false;
    std::shared_ptr<SessionGate> gate;
};

template<typename... Args>
class AsyncCallback {
    struct State : PendingCallback {
        std::function<void(Args...)> success;
        std::function<void(const GsError&)> failure;
        void cancel(const GsError& error) override {
            if (completed) return;
            completed = true;
            auto fn = std::move(failure);
            success = nullptr;
            if (fn) fn(error);
        }
    };
public:
    AsyncCallback() = default;
    AsyncCallback(std::function<void(Args...)> success,
                  std::function<void(const GsError&)> failure) : _state(std::make_shared<State>()) {
        _state->success = std::move(success);
        _state->failure = std::move(failure);
    }
    void success(Args... args) const {
        auto state = _state; // A callback can close its session and release its owner.
        if (!state || state->completed) return;
        if (state->gate && !state->gate->active) {
            state->cancel({GsErrorCode::Cancelled, "Services closed"});
            return;
        }
        state->completed = true;
        auto fn = std::move(state->success);
        state->failure = nullptr;
        if (fn) fn(std::forward<Args>(args)...);
    }
    void failure(const GsError& error) const {
        auto state = _state;
        if (!state || state->completed) return;
        if (state->gate && !state->gate->active) {
            state->cancel({GsErrorCode::Cancelled, "Services closed"});
        } else {
            state->cancel(error);
        }
    }
    explicit operator bool() const { return _state && !_state->completed; }
    void reset() { _state.reset(); }
    std::shared_ptr<PendingCallback> pending() const { return _state; }
private:
    std::shared_ptr<State> _state;
};

template<typename... Args>
class EventDelegate {
public:
    EventDelegate() = default;
    explicit EventDelegate(std::function<void(const Args&...)> callback) : _callback(std::move(callback)) {}
    void invoke(const Args&... args) const {
        auto fn = _callback;
        if (fn && (!_gate || _gate->active)) fn(args...);
    }
    void setGate(std::shared_ptr<SessionGate> gate) { _gate = std::move(gate); }
    explicit operator bool() const { return static_cast<bool>(_callback); }
    void reset() { _callback = nullptr; _gate.reset(); }
private:
    std::function<void(const Args&...)> _callback;
    std::shared_ptr<SessionGate> _gate;
};

using OnComplete = AsyncCallback<>;
#endif

} // namespace cc::Gs

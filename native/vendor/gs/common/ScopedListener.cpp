
#include "vendor/gs/common/ScopedListener.h"
#include "cocos/bindings/jswrapper/SeApi.h"

namespace cc {
scopedListener::scopedListener(se::Object* obj) : _obj(obj) {
    if (_obj) {
        _obj->root();
        _obj->incRef();
    }
}

scopedListener::~scopedListener() {
    if (_obj) {
        _obj->unroot();
        _obj->decRef();
    }
}

scopedListener::scopedListener(const scopedListener& other) : _obj(other._obj) {
    if (_obj) {
        _obj->root();
        _obj->incRef();
    }
}

scopedListener& scopedListener::operator=(const scopedListener& other) {
    if (this != &other) {
        if (_obj) {
            _obj->unroot();
            _obj->decRef();
        }
        _obj = other._obj;
        if (_obj) {
            _obj->root();
            _obj->incRef();
        }
    }
    return *this;
}

scopedListener::scopedListener(scopedListener&& other) noexcept : _obj(other._obj) {
    other._obj = nullptr;
}

scopedListener& scopedListener::operator=(scopedListener&& other) noexcept {
    if (this != &other) {
        if (_obj) {
            _obj->unroot();
            _obj->decRef();
        }
        _obj = other._obj;
        other._obj = nullptr;
    }
    return *this;
}

void scopedListener::reset(se::Object* obj) {
    if (_obj == obj) return;
    if (_obj) {
        _obj->unroot();
        _obj->decRef();
    }
    _obj = obj;
    if (_obj) {
        _obj->root();
        _obj->incRef();
    }
}
}

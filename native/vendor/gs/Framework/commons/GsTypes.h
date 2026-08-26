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

// Value types shared across gs interfaces. Types appearing in JS-exposed
// signatures must also be %include'd in tools/swig-config/gs/gs.i.

#include <cstdint>
#include <string>

#include "base/std/variant.h"

namespace cc::Gs {

// Numeric for Steam's App ID, string for GUID-style IDs (e.g. Epic Product ID).
using AppId = ccstd::variant<uint32_t, std::string>;

// String, not numeric: Steam's 64-bit CSteamID exceeds JS's exact integer range
// (2^53) and Epic uses GUIDs. Matches FriendInfo::userId.
using AccountId = std::string;

} // namespace cc::Gs

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
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "commons/GsCallback.h"
#include "base/RefCounted.h"
#include "base/Ptr.h"

namespace cc::Gs {
class GsSession;
struct FileInfo {
    std::string name;
    uint64_t size = 0; // Bytes.
};
struct QuotaInfo {
    uint64_t totalBytes = 0;
    uint64_t availableBytes = 0;
};
// JSB maps this owned byte buffer to/from Uint8Array, including subarray offsets.
struct FileData { std::vector<uint8_t> bytes; };
#ifdef SWIG
using OnReadFile = AsyncCallbackBase;
using OnFileInfo = AsyncCallbackBase;
using OnFileList = AsyncCallbackBase;
using OnQuota = AsyncCallbackBase;
#else
using OnReadFile = AsyncCallback<FileData>;
using OnFileInfo = AsyncCallback<std::optional<FileInfo>>;
using OnFileList = AsyncCallback<std::vector<FileInfo>>;
using OnQuota = AsyncCallback<QuotaInfo>;
#endif
class IRemoteStorage final : public cc::RefCounted {
public:
    ~IRemoteStorage() override;
    void writeFile(const std::string& name, const FileData& data, OnComplete callback);
    void readFile(const std::string& name, OnReadFile callback);
    void deleteFile(const std::string& name, OnComplete callback);
    void getFileInfo(const std::string& name, OnFileInfo callback);
    void listFiles(OnFileList callback);
    void getQuota(OnQuota callback);
#ifndef SWIG
    explicit IRemoteStorage(cc::IntrusivePtr<GsSession> session);
private:
    cc::IntrusivePtr<GsSession> _session;
#endif
};
} // namespace cc::Gs

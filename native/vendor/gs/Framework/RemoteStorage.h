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
#include <string>
#include <vector>
#include "commons/GsCallback.h"
#include "base/RefCounted.h"
#include "base/Ptr.h"

namespace cc::Gs {

class GsSession;

struct FileInfo {
    std::string FileName;
    int32_t FileSize = 0;
};

struct QuotaInfo {
    bool Success = false;
    uint64_t TotalBytes = 0;
    uint64_t AvailableBytes = 0;
};

struct FileList {
    std::vector<FileInfo> Files;
};

// JSB facade: retains its original session, never the platform implementation.
class IRemoteStorage final : public cc::RefCounted {
public:
    ~IRemoteStorage() override;
    void writeFile(const std::string& fileName, const std::string& data, OnComplete callback);
    void readFile(const std::string& fileName, OnReadFile callback);
    void deleteFile(const std::string& fileName, OnComplete callback);
    bool fileExists(const std::string& fileName);
    int32_t getFileSize(const std::string& fileName);
    int32_t getFileCount();
    FileList getFileList();
    QuotaInfo getQuota();
#ifndef SWIG
    explicit IRemoteStorage(cc::IntrusivePtr<GsSession> session);
private:
    cc::IntrusivePtr<GsSession> _session;
#endif
};

} // namespace cc::Gs

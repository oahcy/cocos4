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
#include <steam_api.h>
#include "../../Framework/backends/RemoteStorageBackend.h"

namespace cc::Gs {
class RemoteStorageSteam : public IRemoteStorageBackend {
public:
    void shutdown() override;
    void writeFile(const std::string& name, const FileData& data, OnComplete callback) override;
    void readFile(const std::string& name, OnReadFile callback) override;
    void deleteFile(const std::string& name, OnComplete callback) override;
    void getFileInfo(const std::string& name, OnFileInfo callback) override;
    void listFiles(OnFileList callback) override;
    void getQuota(OnQuota callback) override;
private:
    void onWriteComplete(RemoteStorageFileWriteAsyncComplete_t* result, bool ioFailure);
    void onReadComplete(RemoteStorageFileReadAsyncComplete_t* result, bool ioFailure);
    CCallResult<RemoteStorageSteam, RemoteStorageFileWriteAsyncComplete_t> _writeCallResult;
    CCallResult<RemoteStorageSteam, RemoteStorageFileReadAsyncComplete_t> _readCallResult;
    OnComplete _pendingWriteCallback;
    OnReadFile _pendingReadCallback;
    FileData _pendingWriteData;
};
} // namespace cc::Gs

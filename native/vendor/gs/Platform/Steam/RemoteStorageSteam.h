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
#include "../../Framework/RemoteStorage.h"
#include "../../Framework/commons/GsComponent.h"

namespace cc::Gs {

class RemoteStorageSteam : public GsComponent<IRemoteStorage> {
public:
    using Super = GsComponent<IRemoteStorage>;

    explicit RemoteStorageSteam(GsServicesCommon& inServices)
        : Super(inServices) {}

    void shutdown() override;

    void writeFile(const std::string& fileName, const std::string& data, OnComplete callback) override;
    void readFile(const std::string& fileName, OnReadFile callback) override;
    void deleteFile(const std::string& fileName, OnComplete callback) override;

    bool fileExists(const std::string& fileName) override;
    int32_t getFileSize(const std::string& fileName) override;
    int32_t getFileCount() override;
    FileList getFileList() override;
    QuotaInfo getQuota() override;

private:
    void onWriteComplete(RemoteStorageFileWriteAsyncComplete_t* pResult, bool bIOFailure);
    void onReadComplete(RemoteStorageFileReadAsyncComplete_t* pResult, bool bIOFailure);

    CCallResult<RemoteStorageSteam, RemoteStorageFileWriteAsyncComplete_t> _writeCallResult;
    OnComplete _pendingWriteCallback;

    CCallResult<RemoteStorageSteam, RemoteStorageFileReadAsyncComplete_t> _readCallResult;
    OnReadFile _pendingReadCallback;
};

} // namespace cc::Gs

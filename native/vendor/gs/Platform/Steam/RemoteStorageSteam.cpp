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

#include "RemoteStorageSteam.h"

#include "base/Log.h"
#include <iostream>

namespace cc::Gs {

void RemoteStorageSteam::shutdown() {
    // Cancel in-flight async calls before SteamAPI_Shutdown. This makes both
    // CCallResult members inert, so their destructors are safe no-ops later.
    // The session has already cancelled requests before backend shutdown.
    _writeCallResult.Cancel();
    if (_pendingWriteCallback) {
        CC_LOG_WARNING("[RemoteStorage] Pending write cancelled during shutdown");
        _pendingWriteCallback.reset();
    }
    _readCallResult.Cancel();
    if (_pendingReadCallback) {
        CC_LOG_WARNING("[RemoteStorage] Pending read cancelled during shutdown");
        _pendingReadCallback.reset();
    }
}

void RemoteStorageSteam::writeFile(const std::string& fileName, const std::string& data, OnComplete callback) {
    // Prevent memory leak: fail the new request if the component is busy.
    // Overwriting a pending callback causes the previous JS promise to hang indefinitely.
    if (_writeCallResult.IsActive()) {
        callback.failure("RemoteStorage is busy. Please wait for the previous write to complete.");
        return;
    }

    SteamAPICall_t apiCall = SteamRemoteStorage()->FileWriteAsync(
        fileName.c_str(), data.data(), static_cast<uint32>(data.size()));

    if (apiCall == k_uAPICallInvalid) {
        callback.failure("FileWriteAsync call failed: " + fileName);
        return;
    }

    _pendingWriteCallback = std::move(callback);
    _writeCallResult.Set(apiCall, this, &RemoteStorageSteam::onWriteComplete);
}

void RemoteStorageSteam::onWriteComplete(RemoteStorageFileWriteAsyncComplete_t* pResult, bool bIOFailure) {
    if (bIOFailure || pResult->m_eResult != k_EResultOK) {
        _pendingWriteCallback.failure(bIOFailure ? "FileWriteAsync IO failure" : "FileWriteAsync failed, EResult=" + std::to_string(pResult->m_eResult));
    } else {
        _pendingWriteCallback.success();
    }
}

void RemoteStorageSteam::readFile(const std::string& fileName, OnReadFile callback) {
    // Prevent memory leak: fail the new request if the component is busy.
    // Overwriting a pending callback causes the previous JS promise to hang indefinitely.
    if (_readCallResult.IsActive()) {
        callback.failure("RemoteStorage is busy. Please wait for the previous read to complete.");
        return;
    }

    int32_t fileSize = SteamRemoteStorage()->GetFileSize(fileName.c_str());
    if (fileSize < 0 || !SteamRemoteStorage()->FileExists(fileName.c_str())) {
        callback.failure("File not found: " + fileName);
        return;
    }
    if (fileSize == 0) {
        callback.success("");
        return;
    }

    SteamAPICall_t apiCall = SteamRemoteStorage()->FileReadAsync(
        fileName.c_str(), 0, static_cast<uint32>(fileSize));

    if (apiCall == k_uAPICallInvalid) {
        callback.failure("FileReadAsync call failed: " + fileName);
        return;
    }

    _pendingReadCallback = std::move(callback);
    _readCallResult.Set(apiCall, this, &RemoteStorageSteam::onReadComplete);
}

void RemoteStorageSteam::onReadComplete(RemoteStorageFileReadAsyncComplete_t* pResult, bool bIOFailure) {
    if (bIOFailure || pResult->m_eResult != k_EResultOK) {
        _pendingReadCallback.failure(bIOFailure ? "FileReadAsync IO failure" : "FileReadAsync failed, EResult=" + std::to_string(pResult->m_eResult));
        return;
    }

    std::string buffer(pResult->m_cubRead, '\0');
    if (SteamRemoteStorage()->FileReadAsyncComplete(
            pResult->m_hFileReadAsync, buffer.data(), pResult->m_cubRead)) {
        _pendingReadCallback.success(buffer);
    } else {
        _pendingReadCallback.failure("FileReadAsyncComplete failed");
    }
}

void RemoteStorageSteam::deleteFile(const std::string& fileName, OnComplete callback) {
    bool ok = SteamRemoteStorage()->FileDelete(fileName.c_str());
    if (ok) {
        callback.success();
    } else {
        callback.failure("FileDelete failed: " + fileName);
    }
}

bool RemoteStorageSteam::fileExists(const std::string& fileName) {
    return SteamRemoteStorage()->FileExists(fileName.c_str());
}

int32_t RemoteStorageSteam::getFileSize(const std::string& fileName) {
    return SteamRemoteStorage()->GetFileSize(fileName.c_str());
}

int32_t RemoteStorageSteam::getFileCount() {
    return SteamRemoteStorage()->GetFileCount();
}

FileList RemoteStorageSteam::getFileList() {
    FileList result;
    int32_t count = SteamRemoteStorage()->GetFileCount();
    result.Files.reserve(count);
    for (int32_t i = 0; i < count; ++i) {
        int32_t fileSize = 0;
        const char* name = SteamRemoteStorage()->GetFileNameAndSize(i, &fileSize);
        FileInfo info;
        info.FileName = name;
        info.FileSize = fileSize;
        result.Files.push_back(std::move(info));
    }
    return result;
}

QuotaInfo RemoteStorageSteam::getQuota() {
    QuotaInfo info;
    info.Success = SteamRemoteStorage()->GetQuota(&info.TotalBytes, &info.AvailableBytes);
    if (!info.Success) {
        info.TotalBytes = 0;
        info.AvailableBytes = 0;
    }
    return info;
}

} // namespace cc::Gs

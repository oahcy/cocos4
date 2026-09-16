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

namespace cc::Gs {
namespace {
bool validName(const std::string& name) { return !name.empty() && name.find('\0') == std::string::npos; }
GsError invalidName() { return {GsErrorCode::InvalidArgument, "File name must be nonempty and contain no NUL"}; }
GsError sdkError(const char* operation, EResult result) {
    return {result == k_EResultFileNotFound ? GsErrorCode::NotFound : GsErrorCode::PlatformError,
            operation, std::to_string(static_cast<int>(result))};
}
}
void RemoteStorageSteam::shutdown() {
    _writeCallResult.Cancel();
    _readCallResult.Cancel();
    _pendingWriteCallback.reset();
    _pendingReadCallback.reset();
    std::vector<uint8_t>().swap(_pendingWriteData.bytes);
}
void RemoteStorageSteam::writeFile(const std::string& name, const FileData& data, OnComplete callback) {
    if (!validName(name)) { callback.failure(invalidName()); return; }
    auto* storage = SteamRemoteStorage();
    if (!storage) { callback.failure({GsErrorCode::NotReady, "Steam storage unavailable"}); return; }
    if (_writeCallResult.IsActive()) { callback.failure({GsErrorCode::Busy, "A file write is already pending"}); return; }
    if (data.bytes.size() > k_unMaxCloudFileChunkSize) { callback.failure({GsErrorCode::InvalidArgument, "File exceeds Steam single-write limit"}); return; }
    _pendingWriteData = data;
    static const uint8_t empty = 0;
    const auto call = storage->FileWriteAsync(name.c_str(), data.bytes.empty() ? &empty : _pendingWriteData.bytes.data(), static_cast<uint32>(data.bytes.size()));
    if (call == k_uAPICallInvalid) {
        std::vector<uint8_t>().swap(_pendingWriteData.bytes);
        callback.failure({GsErrorCode::PlatformError, "FileWriteAsync rejected"}); return;
    }
    _pendingWriteCallback = std::move(callback);
    _writeCallResult.Set(call, this, &RemoteStorageSteam::onWriteComplete);
}
void RemoteStorageSteam::onWriteComplete(RemoteStorageFileWriteAsyncComplete_t* result, bool ioFailure) {
    auto callback = std::move(_pendingWriteCallback);
    _pendingWriteCallback.reset();
    std::vector<uint8_t>().swap(_pendingWriteData.bytes);
    if (ioFailure || !result) callback.failure({GsErrorCode::PlatformError, "File write IO failure"});
    else if (result->m_eResult != k_EResultOK) callback.failure(sdkError("File write failed", result->m_eResult));
    else callback.success();
}
void RemoteStorageSteam::readFile(const std::string& name, OnReadFile callback) {
    if (!validName(name)) { callback.failure(invalidName()); return; }
    auto* storage = SteamRemoteStorage();
    if (!storage) { callback.failure({GsErrorCode::NotReady, "Steam storage unavailable"}); return; }
    if (_readCallResult.IsActive()) { callback.failure({GsErrorCode::Busy, "A file read is already pending"}); return; }
    if (!storage->FileExists(name.c_str())) { callback.failure({GsErrorCode::NotFound, "File not found: " + name}); return; }
    const auto size = storage->GetFileSize(name.c_str());
    if (size < 0 || static_cast<uint32>(size) > k_unMaxCloudFileChunkSize) {
        callback.failure({GsErrorCode::PlatformError, "Invalid or unsupported file size"}); return;
    }
    if (size == 0) { callback.success(FileData{}); return; }
    const auto call = storage->FileReadAsync(name.c_str(), 0, static_cast<uint32>(size));
    if (call == k_uAPICallInvalid) { callback.failure({GsErrorCode::PlatformError, "FileReadAsync rejected"}); return; }
    _pendingReadCallback = std::move(callback);
    _readCallResult.Set(call, this, &RemoteStorageSteam::onReadComplete);
}
void RemoteStorageSteam::onReadComplete(RemoteStorageFileReadAsyncComplete_t* result, bool ioFailure) {
    auto callback = std::move(_pendingReadCallback);
    _pendingReadCallback.reset();
    if (ioFailure || !result) { callback.failure({GsErrorCode::PlatformError, "File read IO failure"}); return; }
    if (result->m_eResult != k_EResultOK) { callback.failure(sdkError("File read failed", result->m_eResult)); return; }
    auto* storage = SteamRemoteStorage();
    if (!storage || result->m_cubRead > k_unMaxCloudFileChunkSize) {
        callback.failure({GsErrorCode::PlatformError, "Invalid file read result"}); return;
    }
    FileData data;
    data.bytes.resize(result->m_cubRead);
    uint8_t empty = 0;
    if (!storage->FileReadAsyncComplete(result->m_hFileReadAsync, data.bytes.empty() ? &empty : data.bytes.data(), result->m_cubRead)) {
        callback.failure({GsErrorCode::PlatformError, "FileReadAsyncComplete failed"}); return;
    }
    callback.success(std::move(data));
}
void RemoteStorageSteam::deleteFile(const std::string& name, OnComplete callback) {
    if (!validName(name)) { callback.failure(invalidName()); return; }
    auto* storage = SteamRemoteStorage();
    if (!storage) { callback.failure({GsErrorCode::NotReady, "Steam storage unavailable"}); return; }
    // Avoid a pending write recreating the file after a successful deletion.
    if (_writeCallResult.IsActive() || _readCallResult.IsActive()) {
        callback.failure({GsErrorCode::Busy, "File transfer is pending"}); return;
    }
    if (!storage->FileExists(name.c_str())) { callback.success(); return; }
    if (storage->FileDelete(name.c_str())) callback.success();
    else callback.failure({GsErrorCode::PlatformError, "FileDelete failed"});
}
void RemoteStorageSteam::getFileInfo(const std::string& name, OnFileInfo callback) {
    if (!validName(name)) { callback.failure(invalidName()); return; }
    auto* storage = SteamRemoteStorage();
    if (!storage) { callback.failure({GsErrorCode::NotReady, "Steam storage unavailable"}); return; }
    if (!storage->FileExists(name.c_str())) { callback.success(std::nullopt); return; }
    const auto size = storage->GetFileSize(name.c_str());
    if (size < 0) { callback.failure({GsErrorCode::PlatformError, "File size query failed"}); return; }
    callback.success(FileInfo{name, static_cast<uint64_t>(size)});
}
void RemoteStorageSteam::listFiles(OnFileList callback) {
    auto* storage = SteamRemoteStorage();
    if (!storage) { callback.failure({GsErrorCode::NotReady, "Steam storage unavailable"}); return; }
    const auto count = storage->GetFileCount();
    if (count < 0) { callback.failure({GsErrorCode::PlatformError, "File list query failed"}); return; }
    std::vector<FileInfo> files;
    for (int32 i = 0; i < count; ++i) {
        int32 size = 0;
        const char* name = storage->GetFileNameAndSize(i, &size);
        if (!name || !*name || size < 0) { callback.failure({GsErrorCode::PlatformError, "File metadata query failed"}); return; }
        files.push_back({name, static_cast<uint64_t>(size)});
    }
    callback.success(std::move(files));
}
void RemoteStorageSteam::getQuota(OnQuota callback) {
    auto* storage = SteamRemoteStorage();
    if (!storage) { callback.failure({GsErrorCode::NotReady, "Steam storage unavailable"}); return; }
    QuotaInfo quota;
    if (!storage->GetQuota(&quota.totalBytes, &quota.availableBytes)) {
        callback.failure({GsErrorCode::PlatformError, "Storage quota query failed"}); return;
    }
    callback.success(quota);
}
} // namespace cc::Gs

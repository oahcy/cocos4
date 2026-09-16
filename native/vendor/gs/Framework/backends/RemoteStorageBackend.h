#pragma once
#include "../RemoteStorage.h"
#include "GsBackend.h"

namespace cc::Gs {
class IRemoteStorageBackend : public GsBackend {
public:
    virtual void writeFile(const std::string& name, const FileData& data, OnComplete callback) = 0;
    virtual void readFile(const std::string& name, OnReadFile callback) = 0;
    virtual void deleteFile(const std::string& name, OnComplete callback) = 0;
    virtual void getFileInfo(const std::string& name, OnFileInfo callback) = 0;
    virtual void listFiles(OnFileList callback) = 0;
    virtual void getQuota(OnQuota callback) {
        callback.failure({GsErrorCode::NotSupported, "Storage quota is not supported"});
    }
};
} // namespace cc::Gs

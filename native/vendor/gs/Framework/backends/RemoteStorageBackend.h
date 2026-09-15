#pragma once

#include "../RemoteStorage.h"
#include "GsBackend.h"

namespace cc::Gs {

class IRemoteStorageBackend : public GsBackend {
public:
    virtual void writeFile(const std::string& fileName, const std::string& data, OnComplete callback) = 0;
    virtual void readFile(const std::string& fileName, OnReadFile callback) = 0;
    virtual void deleteFile(const std::string& fileName, OnComplete callback) = 0;
    virtual bool fileExists(const std::string& fileName) = 0;
    virtual int32_t getFileSize(const std::string& fileName) = 0;
    virtual int32_t getFileCount() = 0;
    virtual FileList getFileList() = 0;
    virtual QuotaInfo getQuota() = 0;
};

} // namespace cc::Gs

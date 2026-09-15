#include "RemoteStorage.h"
#include "commons/GsSession.h"

namespace cc::Gs {

IRemoteStorage::IRemoteStorage(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IRemoteStorage::~IRemoteStorage() = default;

void IRemoteStorage::writeFile(const std::string& fileName, const std::string& data, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        callback.failure("RemoteStorage unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->writeFile(fileName, data, std::move(callback));
}

void IRemoteStorage::readFile(const std::string& fileName, OnReadFile callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        callback.failure("RemoteStorage unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->readFile(fileName, std::move(callback));
}

void IRemoteStorage::deleteFile(const std::string& fileName, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        callback.failure("RemoteStorage unavailable or services closed");
        return;
    }
    _session->track(callback.pending());
    backend->deleteFile(fileName, std::move(callback));
}

bool IRemoteStorage::fileExists(const std::string& fileName) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        return {};
    }
    return backend->fileExists(fileName);
}

int32_t IRemoteStorage::getFileSize(const std::string& fileName) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        return {};
    }
    return backend->getFileSize(fileName);
}

int32_t IRemoteStorage::getFileCount() {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        return {};
    }
    return backend->getFileCount();
}

FileList IRemoteStorage::getFileList() {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        return {};
    }
    return backend->getFileList();
}

QuotaInfo IRemoteStorage::getQuota() {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        return {};
    }
    return backend->getQuota();
}

} // namespace cc::Gs

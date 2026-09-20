#include "RemoteStorage.h"
#include "commons/GsSession.h"

namespace cc::Gs {
IRemoteStorage::IRemoteStorage(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IRemoteStorage::~IRemoteStorage() = default;

void IRemoteStorage::writeFile(const std::string& name, const FileData& data, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->remoteStorage(), callback, "Remote storage", std::chrono::seconds(60));
    if (!backend) return;
    backend->writeFile(name, data, std::move(callback));
}

void IRemoteStorage::readFile(const std::string& name, OnReadFile callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->remoteStorage(), callback, "Remote storage", std::chrono::seconds(60));
    if (!backend) return;
    backend->readFile(name, std::move(callback));
}

void IRemoteStorage::deleteFile(const std::string& name, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->remoteStorage(), callback, "Remote storage");
    if (!backend) return;
    backend->deleteFile(name, std::move(callback));
}

void IRemoteStorage::getFileInfo(const std::string& name, OnFileInfo callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->remoteStorage(), callback, "Remote storage");
    if (!backend) return;
    backend->getFileInfo(name, std::move(callback));
}

void IRemoteStorage::listFiles(OnFileList callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->remoteStorage(), callback, "Remote storage");
    if (!backend) return;
    backend->listFiles(std::move(callback));
}

void IRemoteStorage::getQuota(OnQuota callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->prepare(_session->remoteStorage(), callback, "Remote storage");
    if (!backend) return;
    backend->getQuota(std::move(callback));
}
} // namespace cc::Gs

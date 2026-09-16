#include "RemoteStorage.h"
#include "commons/GsSession.h"

namespace cc::Gs {
IRemoteStorage::IRemoteStorage(IntrusivePtr<GsSession> session) : _session(std::move(session)) {}
IRemoteStorage::~IRemoteStorage() = default;

void IRemoteStorage::writeFile(const std::string& name, const FileData& data, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Remote storage unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->writeFile(name, data, std::move(callback));
}

void IRemoteStorage::readFile(const std::string& name, OnReadFile callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Remote storage unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->readFile(name, std::move(callback));
}

void IRemoteStorage::deleteFile(const std::string& name, OnComplete callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Remote storage unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->deleteFile(name, std::move(callback));
}

void IRemoteStorage::getFileInfo(const std::string& name, OnFileInfo callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Remote storage unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->getFileInfo(name, std::move(callback));
}

void IRemoteStorage::listFiles(OnFileList callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Remote storage unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->listFiles(std::move(callback));
}

void IRemoteStorage::getQuota(OnQuota callback) {
    GsSession::Dispatch dispatch(*_session);
    auto* backend = _session->remoteStorage();
    if (!backend) {
        const auto code = _session->isClosed() || _session->isClosing() ? GsErrorCode::Cancelled
            : (_session->isActive() ? GsErrorCode::NotSupported : GsErrorCode::NotReady);
        callback.failure({code, "Remote storage unavailable in this session"});
        return;
    }
    _session->track(callback.pending());
    backend->getQuota(std::move(callback));
}
} // namespace cc::Gs

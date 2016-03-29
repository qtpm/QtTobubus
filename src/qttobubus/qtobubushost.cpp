#include <QDir>
#include <QEventLoop>
#include <QReadLocker>
#include <QWriteLocker>
#include <QLocalServer>
#include <QtConcurrent>
#include "qtobubushost.h"
#include "private/message.h"
#include "private/proxy.h"
#include "private/hostsocketthread.h"
#include "private/sessionmanager.h"

static void write(HostSocketThread *socket, MessageType type, quint32 sessionID)
{
    QMetaObject::invokeMethod(socket, "write", Qt::AutoConnection,
        Q_ARG(QByteArray, Message::archive(type, sessionID)));
}

static void write(HostSocketThread *socket, MessageType type, quint32 sessionID, const QByteArray &body)
{
    QMetaObject::invokeMethod(socket, "write", Qt::AutoConnection,
        Q_ARG(QByteArray, Message::archive(Unpublish, sessionID, body)));
}

static void write(HostSocketThread *socket, const QByteArray &body)
{
    QMetaObject::invokeMethod(socket, "write", Qt::AutoConnection, Q_ARG(QByteArray, body));
}


QString QTobubusHost::getPipeName() const { return _pipeName; }

QTobubusHost::QTobubusHost(const QString& pipeName, QObject* parent)
	: QObject(parent)
	, _pipeName(pipeName)
{
    Call::RegisterStatus();
    this->_sessions = new SessionManager();

    this->_server = new QLocalServer();
    connect(this->_server, &QLocalServer::newConnection, [this]{
        QLocalSocket* socket = this->_server->nextPendingConnection();
        new HostSocketThread(this, socket);
    });
}

QTobubusHost::~QTobubusHost()
{
	this->close();
    this->_server->close();
    this->_server->deleteLater();
    this->_server = nullptr;
    delete this->_sessions;
    this->_sessions = nullptr;
}

bool QTobubusHost::confirmPath(const QString& path) const
{
    if (this->_localObjectMap.contains(path)) {
        return true;
    }
    return this->_pluginReservedSpaces.contains(path);
}

void QTobubusHost::listen()
{
    this->close();
#if !defined(__WIN32__)
    auto tmp = QDir::temp();
    tmp.remove(this->_pipeName);
#endif
    this->_server->listen(this->_pipeName);
    qDebug() << this->_server->fullServerName();
}

void QTobubusHost::close()
{
    if (this->_server->isListening()) {
        for (auto& socket : this->_id2socket) {
            QMetaObject::invokeMethod(socket, "closeAndDelete", Qt::QueuedConnection);
        }
        this->_server->close();
    }
}

bool QTobubusHost::unregister(const QString& pluginID)
{
    this->_lock.lockForWrite();
    auto socket = this->_id2socket[pluginID];
    if (socket) {
        this->_unregister(socket);
    }
    this->_lock.unlock();
    if (!socket) {
        return false;
    }
    this->_sendCloseClientMessage(socket);
    return true;
}

void QTobubusHost::publish(const QString& path, QSharedPointer<QObject> instance)
{
    auto proxy = new Proxy(instance);
    QWriteLocker locker(&this->_lock);
    auto existingProxy = this->_localObjectMap[path];
    if (existingProxy) {
        delete existingProxy;
    }
    qDebug() << "publish: " << path;
    this->_localObjectMap[path] = proxy;
}

AliasObject<QTobubusHost> QTobubusHost::operator[](const QString path)
{
    return AliasObject<QTobubusHost>(this, path);
}

void QTobubusHost::_unregister(HostSocketThread *socket, bool lock)
{
    if (lock) {
        this->_lock.lockForWrite();
    }
    QStringList removeKeys;
    for (auto i = this->_pluginReservedSpaces.constBegin();
         i != this->_pluginReservedSpaces.constEnd(); i++) {
        if (i.value() == socket) {
            removeKeys.append(i.key());
        }
    }
    for (auto& key : removeKeys) {
        this->_pluginReservedSpaces.remove(key);
    }
    auto pluginID = this->_socket2id.take(socket);
    this->_id2socket.take(pluginID);
    if (lock) {
        this->_lock.unlock();
    }
}

void QTobubusHost::_sendCloseClientMessage(HostSocketThread *socket)
{
    auto sessionID = this->_sessions->getUniqueSessionID();
    write(socket, CloseClient, sessionID);
    this->_sessions->close(sessionID);
}

bool QTobubusHost::_onReceiveMessage(Message *msg, HostSocketThread *socket)
{
    // This method is run on the socket thread
    qDebug() << "receiveMessage" << msg->type;
    switch (msg->type) {
        case ResultOK:
        case ResultNG:
        case ReturnMethod:
            this->_sessions->send(msg->id, msg);
            break;
        case ConnectClient: {
            qDebug() << "connect";
            auto pluginID = QString(msg->body);
            QWriteLocker lock(&this->_lock);
            auto existingSocket = this->_id2socket.take(pluginID);
            if (existingSocket) {
                this->_unregister(existingSocket);
                this->_sendCloseClientMessage(existingSocket);
            }
            write(socket, ResultOK, msg->id);
            this->_id2socket[pluginID] = socket;
            this->_socket2id[socket] = pluginID;
            delete msg;
            emit clientConnected(pluginID);
        } break;
        case Publish: {
            auto path = QString(msg->body);
            QWriteLocker lock(&this->_lock);
            auto existingSocket = this->_pluginReservedSpaces.take(path);
            if (existingSocket) {
                auto sessionID = this->_sessions->getUniqueSessionID();
                write(existingSocket, Unpublish, sessionID, msg->body);
                this->_sessions->receiveAndClose(sessionID);
            }
            write(socket, ResultOK, msg->id);
            this->_pluginReservedSpaces[path] = socket;
            delete msg;
            emit objectPublished(this->_socket2id[socket], path);
        } break;
        case CallMethod: {
            qDebug() << __LINE__;
            QtConcurrent::run([this, msg, socket] {
                qDebug() << __LINE__;
                QScopedPointer<MethodCall> method(MethodCall::parse(msg->body));
                qDebug() << __LINE__;
                auto object = this->_localObjectMap[method->path];
                qDebug() << __LINE__;
                if (!object) {
                    qDebug() << __LINE__;
                    write(socket, ResultObjectNotFound, msg->id);
                    qDebug() << __LINE__;
                } else if (!object->hasMethod(method->method)) {
                    qDebug() << __LINE__;
                    write(socket, ResultMethodNotFound, msg->id);
                    qDebug() << __LINE__;
                } else {
                    qDebug() << __LINE__;
                    QVariantList result;
                    qDebug() << __LINE__;
                    result.append(object->call(method->method, method->params));
                    qDebug() << __LINE__;
                    write(socket, MethodCall::archive(ReturnMethod, msg->id, "", "", result));
                    qDebug() << __LINE__;
                    emit methodCalled(this->_socket2id[socket], method->path, method->method, method->params);
                    qDebug() << __LINE__;
                }
                qDebug() << __LINE__;
                delete msg;
                qDebug() << __LINE__;
            });
        } break;
        case CloseClient: {
            auto socketID = this->_socket2id[socket];
            if (socketID == "") {
                write(socket, ResultNG, msg->id);
            } else {
                QWriteLocker lock(&this->_lock);
                QStringList removeTargetPaths;
                for (auto i = this->_pluginReservedSpaces.constBegin();
                     i != this->_pluginReservedSpaces.constEnd(); i++) {
                    if (i.value() == socket) {
                        removeTargetPaths.append(i.key());
                    }
                }
                for (auto& path : removeTargetPaths) {
                    this->_pluginReservedSpaces.remove(path);
                }
                this->_id2socket.remove(socketID);
                write(socket, ResultOK, msg->id);
                delete msg;
            }
        } break;
        case ConfirmPath: {
            auto msgType = (this->_localObjectMap.contains(QString(msg->body)))
                ? ResultOK
                : ResultObjectNotFound;
            write(socket, msgType, msg->id);
            delete msg;
        } break;
        default:
            delete msg;
            return false;
    }
    return true;

}

void QTobubusHost::apply(const QString& path, const QString& methodName, QVariantList params)
{
    qDebug() << "apply: " << path << ", " << methodName << ", " << params;
    this->_lock.lockForRead();
    auto proxy = this->_localObjectMap[path];
    if (proxy) {
        qDebug() << "found in host";
        this->_lock.unlock();
        if (!proxy->hasMethod(methodName)) {
            emit methodResult(path, methodName, QVariantList{}, Call::InvalidMethod);
        } else {
            emit methodResult(path, methodName, QVariantList{proxy->call(methodName, params)}, Call::Success);
        }
        return;
    }
    qDebug() << "not found in host";
    auto socket = this->_pluginReservedSpaces[path];
    this->_lock.unlock();
    if (socket) {
        auto sessionID = this->_sessions->getUniqueSessionID();
        auto data = MethodCall::archive(CallMethod, sessionID, path, methodName, params);
        if (data.size() == 0) {
            emit methodResult(path, methodName, QVariantList{}, Call::SerializeError);
            return;
        }
        qDebug() << "socket write for method call";
        write(socket, data);
        qDebug() << "wait receive for method call";
        this->_sessions->receive(sessionID, [this, path, methodName, sessionID](Message *message) {
            switch (message->type) {
                case ReturnMethod: {
                    qDebug() << "OK";
                    QScopedPointer<MethodCall> method(MethodCall::parse(message->body));
                    emit methodResult(path, methodName, method->params, Call::Success);
                } break;
                case ResultObjectNotFound:
                    qDebug() << "ObjectNotFound";
                    emit methodResult(path, methodName, QVariantList{}, Call::InvalidPath);
                    break;
                case ResultMethodNotFound:
                    qDebug() << "MethodNotFound";
                    emit methodResult(path, methodName, QVariantList{}, Call::InvalidMethod);
                    break;
                case ResultMethodError:
                    qDebug() << "Method Error";
                    emit methodResult(path, methodName, QVariantList{}, Call::RuntimeError);
                    break;
                default:
                    emit methodResult(path, methodName, QVariantList{}, Call::RemoteError);
                    qDebug() << "Other:" << message->type;
            }
        });
    } else {
        emit methodResult(path, methodName, QVariantList{}, Call::SocketClosed);
    }
}

#include "moc_qtobubushost.cpp"

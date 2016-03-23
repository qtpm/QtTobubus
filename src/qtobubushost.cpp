#include "qtobubushost.h"
#include "private/message.h"
#include "private/proxy.h"
#include "private/sessionmanager.h"
#include <QEventLoop>
#include <QReadLocker>
#include <QWriteLocker>
#include <QtConcurrent>

QString QTobubusHost::getPipeName() const { return _pipeName; }

QTobubusHost::QTobubusHost(const QString& pipeName, QObject* parent)
	: QObject(parent)
	, _pipeName(pipeName)
	, _server(nullptr)
{
	this->_sessions = new SessionManager();
}

QTobubusHost::~QTobubusHost()
{
	this->close();
	delete this->_sessions;
	this->_sessions = nullptr;
	if (this->_server) {
		delete this->_server;
		this->_server = nullptr;
	}
}

QIODevice* QTobubusHost::getSocket(const QString& pluginID) const
{
	return this->_sockets[pluginID];
}

QString QTobubusHost::getPluginID(const QIODevice* socket) const
{
	QReadLocker locker(&this->_lock);
	for (auto i = this->_sockets.constBegin(); i != this->_sockets.constEnd(); i++) {
		if (i.value() == socket) {
			return i.key();
		}
	}
	return "";
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
	QWriteLocker locker(&this->_lock);
    this->_server = new QLocalServer();
	this->_server->listen(this->_pipeName);
	connect(this->_server, &QLocalServer::newConnection, [this]() {
		QtConcurrent::run(
			[this] { this->listenAndServeTo(this->_server->nextPendingConnection()); });
	});
}

void QTobubusHost::listenAndServe()
{
	this->listen();
	QEventLoop eventLoop;
	connect(this->_server, SIGNAL(destroyed(QObject*)), &eventLoop, SLOT(quit()));
}

void QTobubusHost::close()
{
	QWriteLocker locker(&this->_lock);
	if (!this->_server) {
		return;
	}
	for (auto& socket : this->_sockets) {
		socket->close();
		delete socket;
	}
	this->_pluginReservedSpaces.clear();
	this->_sockets.clear();
	this->_server->close();
	this->_server = nullptr;
}

void QTobubusHost::_unregister(QIODevice* socket)
{
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
}

bool QTobubusHost::_sendCloseClientMessage(QIODevice* socket)
{
	auto sessionID = this->_sessions->getUniqueSessionID();
	socket->write(Message::archive(CloseClient, sessionID));
	auto message = this->_sessions->receiveAndClose(sessionID);
	delete socket;
	return message->type == ResultOK;
}

bool QTobubusHost::unregister(const QString& pluginID)
{
	this->_lock.lockForWrite();
	auto socket = this->_sockets.take(pluginID);
	if (socket) {
		_unregister(socket);
	}
	this->_lock.unlock();
	if (!socket) {
		return false;
	}
	return _sendCloseClientMessage(socket);
}

void QTobubusHost::publish(const QString& path, QSharedPointer<QObject> instance)
{
	auto proxy = new Proxy(instance);
	QWriteLocker locker(&this->_lock);
	auto existingProxy = this->_localObjectMap[path];
	if (existingProxy) {
		delete existingProxy;
	}
	this->_localObjectMap[path] = proxy;
}

AliasObject<QTobubusHost> QTobubusHost::operator[](const QString path)
{
    return AliasObject<QTobubusHost>(this, path);
}

QVariant QTobubusHost::apply(const QString& path, const QString& method, QVariantList params)
{
	Call::Status status;
	return this->apply(path, method, &status, params);
}

QVariant QTobubusHost::apply(
	const QString& path, const QString& method, Call::Status* status, QVariantList params)
{
	this->_lock.lockForRead();
	auto proxy = this->_localObjectMap[path];
	if (proxy) {
		this->_lock.unlock();
		if (proxy->hasMethod(method)) {
			*status = Call::Success;
			return proxy->call(method, params);
		}
		*status = Call::InvalidPath;
		return QVariant();
	}
	auto socket = this->_pluginReservedSpaces[path];
	this->_lock.unlock();
	if (socket) {
		auto sessionID = this->_sessions->getUniqueSessionID();
		auto data = MethodCall::archive(CallMethod, sessionID, path, method, params);
		if (data.size() == 0) {
			*status = Call::SerializeError;
			return QVariant();
		}
		auto size = socket->write(data);
		if (size != data.size()) {
			*status = Call::CommunicationError;
			return QVariant();
		}
		auto message = this->_sessions->receiveAndClose(sessionID);
		switch (message->type) {
			case ResultOK: {
				*status = Call::Success;
				QScopedPointer<MethodCall> method(MethodCall::parse(message->body));
				return method->params;
			}
			case ResultObjectNotFound:
				*status = Call::InvalidPath;
				break;
			case ResultMethodNotFound:
				*status = Call::InvalidMethod;
				break;
			default:
				*status = Call::RemoteError;
		}
	} else {
		*status = Call::SocketClosed;
	}
	return QVariant();
}

bool QTobubusHost::receiveMessage(QIODevice* socket)
{
	auto msg = Message::parse(*socket);
	if (msg == nullptr) {
		return false;
	}
	switch (msg->type) {
		case ResultOK:
		case ResultNG:
		case ReturnMethod: {
			auto channel = this->_sessions->getChannelOfSessionID(msg->id);
			channel->send(msg);
		} break;
		case ConnectClient: {
			auto pluginID = QString(msg->body);
			QWriteLocker lock(&this->_lock);
			auto existingSocket = this->_sockets.take(pluginID);
			if (existingSocket) {
                this->_unregister(existingSocket);
                this->_sendCloseClientMessage(existingSocket);
			}
			socket->write(Message::archive(ResultOK, msg->id));
			this->_sockets[pluginID] = socket;
			delete msg;
		} break;
		case Publish: {
			auto path = QString(msg->body);
			QWriteLocker lock(&this->_lock);
			auto existingSocket = this->_pluginReservedSpaces.take(path);
			if (existingSocket) {
				auto sessionID = this->_sessions->getUniqueSessionID();
				existingSocket->write(Message::archive(Unpublish, sessionID, msg->body));
				this->_sessions->receiveAndClose(sessionID);
			}
			socket->write(Message::archive(ResultOK, msg->id));
			this->_pluginReservedSpaces[path] = socket;
			delete msg;
		} break;
		case CallMethod:
			QtConcurrent::run([this, msg, socket] {
				QScopedPointer<MethodCall> method(MethodCall::parse(msg->body));
				auto object = this->_localObjectMap[method->path];
				if (!object) {
					socket->write(Message::archive(ResultObjectNotFound, msg->id));
				} else if (!object->hasMethod(method->method)) {
					socket->write(Message::archive(ResultMethodNotFound, msg->id));
				} else {
					QVariantList result;
					result.append(object->call(method->method, method->params));
					socket->write(MethodCall::archive(ReturnMethod, msg->id, "", "", result));
				}
				delete msg;
			});
			break;
		case CloseClient: {
			auto socketID = this->getPluginID(socket);
			if (socketID == "") {
				socket->write(Message::archive(ResultNG, msg->id));
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
				this->_sockets.remove(socketID);
				socket->write(Message::archive(ResultOK, msg->id));
				delete msg;
			}
		} break;
		case ConfirmPath: {
			auto msgType = (this->_localObjectMap.contains(QString(msg->body)))
				? ResultOK
				: ResultObjectNotFound;
			socket->write(Message::archive(msgType, msg->id));
			delete msg;
		} break;
		default:
			delete msg;
			return false;
	}
	return true;
}

void QTobubusHost::listenAndServeTo(QIODevice* socket)
{
	while (true) {
		auto ok = this->receiveMessage(socket);
		if (!ok) {
			return;
		}
	}
}

#include "moc_qtobubushost.cpp"

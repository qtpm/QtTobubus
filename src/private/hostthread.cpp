#include "hostthread.h"
#include "proxy.h"
#include "qtobubushost.h"
#include "sessionmanager.h"
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>

HostThread::HostThread(
	QTobubusHost* interface, SessionManager* sessions, const QString& pipeName, QObject* parent)
	: QObject(parent)
	, _interface(interface)
	, _sessions(sessions)
{
#if !defined(__WIN32__)
	auto tmp = QDir::temp();
	tmp.remove(pipeName);
#endif

	this->_thread = new QThread;
	this->moveToThread(this->_thread);
	connect(this->_thread, &QThread::started, [this, &pipeName] {
		this->_server = new QLocalServer();
		connect(this->_server, &QLocalServer::newConnection, [this]() {
			auto socket = this->_server->nextPendingConnection();
			connect(socket, SIGNAL(readyRead()), this, SLOT(onReceiveMessage()));
			connect(socket, SIGNAL(aboutToClose()), this, SLOT(onUnregister()));
		});
		this->_server->listen(pipeName);
		qDebug() << this->_server->fullServerName();
	});

	this->_thread->start();
}

HostThread::~HostThread()
{
	if (this->_thread) {
		for (auto& socket : this->_id2socket) {
			socket->close();
			delete socket;
		}
		this->_server->close();
		emit this->_server->deleteLater();
		this->_thread->exit();
		emit this->_thread->deleteLater();
		this->_server = nullptr;
		this->_thread = nullptr;
	}
}

QIODevice* HostThread::getSocket(const QString& pluginID) const
{
	return this->_id2socket[pluginID];
}

QString HostThread::getPluginID(const QIODevice* socket) const { return this->_socket2id[socket]; }

bool HostThread::confirmPath(const QString& path) const
{
	if (this->_localObjectMap.contains(path)) {
		return true;
	}
	return this->_pluginReservedSpaces.contains(path);
}

bool HostThread::unregister(const QString& pluginID)
{
	this->_lock.lockForWrite();
	auto socket = this->_id2socket[pluginID];
	if (socket) {
		_unregister(socket);
	}
	this->_lock.unlock();
	if (!socket) {
		return false;
	}
	return this->_sendCloseClientMessage(socket);
}

void HostThread::publish(const QString& path, QSharedPointer<QObject> instance)
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

QVariant HostThread::apply(
	const QString& path, const QString& method, Call::Status* status, QVariantList params)
{
	qDebug() << "apply: " << path << ", " << method << ", " << params;
	this->_lock.lockForRead();
	auto proxy = this->_localObjectMap[path];
	if (proxy) {
		qDebug() << "found in host";
		this->_lock.unlock();
		if (proxy->hasMethod(method)) {
			*status = Call::Success;
			return proxy->call(method, params);
		}
		*status = Call::InvalidPath;
		return QVariant();
	}
	qDebug() << "not found in host";
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

bool HostThread::onReceiveMessage()
{
	QIODevice* socket = dynamic_cast<QIODevice*>(this->sender());
	auto msg = Message::parse(*socket);
	if (msg == nullptr) {
		return false;
	}
	qDebug() << "receiveMessage" << msg->type;
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
			auto existingSocket = this->_id2socket.take(pluginID);
			if (existingSocket) {
				this->_unregister(existingSocket);
				this->_sendCloseClientMessage(existingSocket);
			}
			socket->write(Message::archive(ResultOK, msg->id));
			this->_id2socket[pluginID] = socket;
			this->_socket2id[socket] = pluginID;
			delete msg;
			QMetaObject::invokeMethod(
				this->_interface, "clientConnected", Q_ARG(QString, pluginID));
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
			QMetaObject::invokeMethod(this->_interface, "objectPublished",
				Q_ARG(QString, this->_socket2id[socket]), Q_ARG(QString, path));
		} break;
		case CallMethod: {
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
				QMetaObject::invokeMethod(this->_interface, "methodCalled",
					Q_ARG(QString, this->_socket2id[socket]), Q_ARG(QString, method->path),
					Q_ARG(QString, method->method), Q_ARG(QVariantList, method->params));
			}
			delete msg;
		} break;
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
				this->_id2socket.remove(socketID);
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

void HostThread::onUnregister()
{
	QIODevice* socket = dynamic_cast<QIODevice*>(this->sender());
	QWriteLocker locker(&this->_lock);
	this->_unregister(socket);
}

void HostThread::_unregister(QIODevice* socket)
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
	auto pluginID = this->_socket2id.take(socket);
	this->_id2socket.take(pluginID);
}

bool HostThread::_sendCloseClientMessage(QIODevice* socket)
{
	auto sessionID = this->_sessions->getUniqueSessionID();
	socket->write(Message::archive(CloseClient, sessionID));
	auto message = this->_sessions->receiveAndClose(sessionID);
	delete socket;
	return message->type == ResultOK;
}

#include "moc_hostthread.cpp"

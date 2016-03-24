#include "qtobubushost.h"
#include "private/hostthread.h"
#include "private/message.h"
#include "private/proxy.h"
#include "private/sessionmanager.h"
#include <QDir>
#include <QEventLoop>
#include <QReadLocker>
#include <QWriteLocker>

QString QTobubusHost::getPipeName() const { return _pipeName; }

QTobubusHost::QTobubusHost(const QString& pipeName, QObject* parent)
	: QObject(parent)
	, _pipeName(pipeName)
	, _thread(nullptr)
{
	this->_sessions = new SessionManager();
}

QTobubusHost::~QTobubusHost()
{
	this->close();
	delete this->_sessions;
	this->_sessions = nullptr;
}

QIODevice* QTobubusHost::getSocket(const QString& pluginID) const
{
	return this->_thread->getSocket(pluginID);
}

QString QTobubusHost::getPluginID(const QIODevice* socket) const
{
	return this->_thread->getPluginID(socket);
}

bool QTobubusHost::confirmPath(const QString& path) const
{
	return this->_thread->confirmPath(path);
}

void QTobubusHost::listen()
{
	if (this->_thread) {
		delete this->_thread;
	}
	this->_thread = new HostThread(this, this->_sessions, this->_pipeName);
}

void QTobubusHost::close()
{
	if (this->_thread != nullptr) {
		this->_thread->deleteLater();
		this->_thread = nullptr;
	}
}

bool QTobubusHost::unregister(const QString& pluginID)
{
	return this->_thread->unregister(pluginID);
}

void QTobubusHost::publish(const QString& path, QSharedPointer<QObject> instance)
{
	this->_thread->publish(path, instance);
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
	QVariant result;
	QMetaObject::invokeMethod(this->_thread, "apply", Qt::DirectConnection,
		Q_RETURN_ARG(QVariant, result), Q_ARG(QString, path), Q_ARG(QString, method),
		Q_ARG(Call::Status*, status), Q_ARG(QVariantList, params));
	return result;
}

#include "moc_qtobubushost.cpp"

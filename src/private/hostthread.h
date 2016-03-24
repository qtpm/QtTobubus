#ifndef HOSTTHREAD_H
#define HOSTTHREAD_H

#include "aliasobject.h"
#include "status.h"
#include <QIODevice>
#include <QLocalServer>
#include <QMap>
#include <QObject>
#include <QReadWriteLock>
#include <QThread>

class SessionManager;
class Proxy;

class HostThread : public QObject
{
	Q_OBJECT

	QTobubusHost* _interface;
	SessionManager* _sessions;
	QLocalServer* _server;
	QThread* _thread;
	mutable QReadWriteLock _lock;

	QMap<QString, QIODevice*> _pluginReservedSpaces;
	QMap<QString, Proxy*> _localObjectMap;
	QMap<QString, QIODevice*> _id2socket;
	QMap<const QIODevice*, QString> _socket2id;

public:
	explicit HostThread(QTobubusHost* interface, SessionManager* sessions, const QString& pipeName,
		QObject* parent = 0);
	virtual ~HostThread();

	QIODevice* getSocket(const QString& pluginID) const;
	QString getPluginID(const QIODevice* socket) const;
	bool confirmPath(const QString& path) const;

public slots:
	bool unregister(const QString& pluginID);
	void publish(const QString& path, QSharedPointer<QObject> instance);

	QVariant apply(
		const QString& path, const QString& method, Call::Status* status, QVariantList params);

private slots:
	bool onReceiveMessage();
	void onUnregister();

private:
	void _unregister(QIODevice* socket);
	bool _sendCloseClientMessage(QIODevice* socket);
};

#endif // HOSTTHREAD_H

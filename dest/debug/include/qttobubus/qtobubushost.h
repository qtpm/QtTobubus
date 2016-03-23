#ifndef QTOBUBUSHOST_H
#define QTOBUBUSHOST_H

#include "qttobubus_global.h"
#include <QObject>
#include <QLocalServer>
#include <QIODevice>
#include <QReadWriteLock>
#include <QScopedPointer>
#include <QMap>
#include <QSharedPointer>
#include "aliasobject.h"
#include "status.h"

class SessionManager;
class Proxy;

class QTTOBUBUSSHARED_EXPORT QTobubusHost : public QObject {
    Q_OBJECT

    QString _pipeName;
    QLocalServer* _server;
    SessionManager* _sessions;
    mutable QReadWriteLock _lock;

    QMap<QString, QIODevice*> _pluginReservedSpaces;
    QMap<QString, Proxy*> _localObjectMap;
    QMap<QString, QIODevice*> _sockets;

public:
    explicit QTobubusHost(const QString& pipeName, QObject *parent = 0);
    virtual ~QTobubusHost();

    QIODevice* getSocket(const QString& pluginID) const;
    QString getPluginID(const QIODevice* socket) const;
    bool confirmPath(const QString& path) const;
    QString getPipeName() const;

    void listen();
    void listenAndServe();
    void close();
    bool unregister(const QString& pluginID);
    void publish(const QString& path, QSharedPointer<QObject> instance);

    AliasObject<QTobubusHost> operator[](const QString path);
    QVariant apply(const QString& path, const QString& method, QVariantList params);
    QVariant apply(const QString& path, const QString& method, Call::Status* status, QVariantList params);

private:
    bool receiveMessage(QIODevice* socket);
    void listenAndServeTo(QIODevice *socket);
    void _unregister(QIODevice *socket);
    bool _sendCloseClientMessage(QIODevice *socket);
};

#endif // QTOBUBUSHOST_H


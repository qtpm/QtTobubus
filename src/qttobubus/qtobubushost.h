#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QReadWriteLock>
#include "aliasobject.h"
#include "status.h"

class CallbackManager;
class SessionManager;
class Proxy;
class QIODevice;
class QLocalServer;
class HostSocketThread;
struct Message;

class QTobubusHost : public QObject {
    Q_OBJECT

    SessionManager* _sessions;
    QLocalServer* _server;
    QString _pipeName;
    mutable QReadWriteLock _lock;

    QMap<QString, HostSocketThread*> _pluginReservedSpaces;
    QMap<QString, Proxy*> _localObjectMap;
    QMap<QString, HostSocketThread*> _id2socket;
    QMap<const HostSocketThread*, QString> _socket2id;

public:
    explicit QTobubusHost(const QString& pipeName, QObject *parent = 0);
    virtual ~QTobubusHost();

    bool confirmPath(const QString& path) const;
    QString getPipeName() const;

    void listen();
    void close();
    bool unregister(const QString& pluginID);
    void publish(const QString& path, QSharedPointer<QObject> instance);

    AliasObject<QTobubusHost> operator[](const QString path);
    void apply(const QString& path, const QString& method, QVariantList params);

    friend class HostSocketThread;
signals:
    void clientConnected(QString id);
    void objectPublished(QString id, QString path);
    void methodResult(QString path, QString method, QVariantList result, Call::Status status);
    void methodCalled(QString id, QString path, QString method, QVariantList params);

private:
    void _unregister(HostSocketThread* socket, bool lock=false);
    void _sendCloseClientMessage(HostSocketThread* socket);
    bool _onReceiveMessage(Message* msg, HostSocketThread* socket);
};

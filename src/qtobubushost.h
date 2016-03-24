#ifndef QTOBUBUSHOST_H
#define QTOBUBUSHOST_H

#include <QObject>
#include <QSharedPointer>
#include "aliasobject.h"
#include "status.h"

class HostThread;
class SessionManager;
class QIODevice;

class QTobubusHost : public QObject {
    Q_OBJECT

    SessionManager* _sessions;
    QString _pipeName;
    HostThread* _thread;
public:
    explicit QTobubusHost(const QString& pipeName, QObject *parent = 0);
    virtual ~QTobubusHost();

    QIODevice* getSocket(const QString& pluginID) const;
    QString getPluginID(const QIODevice* socket) const;
    bool confirmPath(const QString& path) const;
    QString getPipeName() const;

    void listen();
    void close();
    bool unregister(const QString& pluginID);
    void publish(const QString& path, QSharedPointer<QObject> instance);

    AliasObject<QTobubusHost> operator[](const QString path);
    QVariant apply(const QString& path, const QString& method, QVariantList params);
    QVariant apply(const QString& path, const QString& method, Call::Status* status, QVariantList params);

signals:
    void clientConnected(QString id);
    void objectPublished(QString id, QString path);
    void methodCalled(QString id, QString path, QString method, QVariantList params);
};

#endif // QTOBUBUSHOST_H


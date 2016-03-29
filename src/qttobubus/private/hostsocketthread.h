#pragma once

#include <QObject>

class QThread;
class QTobubusHost;
class QIODevice;

class HostSocketThread : public QObject {
    Q_OBJECT

    QTobubusHost *_host;
    QThread* _thread;
    QIODevice *_socket;

public:
    explicit HostSocketThread(QTobubusHost* host, QIODevice* socket, QObject *parent = 0);
    virtual ~HostSocketThread();

signals:
    void aboutToClose();

public slots:
    void onReceiveMessage();
    void onUnregister();
    qint64 write(const QByteArray& data);
    void closeAndDelete();
};

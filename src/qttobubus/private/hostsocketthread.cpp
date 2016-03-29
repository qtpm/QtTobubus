#include "hostsocketthread.h"
#include "../qtobubushost.h"
#include "message.h"
#include <QThread>

HostSocketThread::HostSocketThread(QTobubusHost* host, QIODevice *socket, QObject *parent)
    : QObject(parent)
    , _host(host)
    , _socket(socket)
{
    this->_thread = new QThread;
    this->moveToThread(this->_thread);
    this->_socket->setParent(nullptr);
    this->_socket->moveToThread(this->_thread);

    connect(this->_thread, &QThread::started, [this, socket] {
        connect(socket, SIGNAL(readyRead()), this, SLOT(onReceiveMessage()));
        connect(socket, SIGNAL(aboutToClose()), this, SLOT(onUnregister()));
    });

    this->_thread->start();
}

HostSocketThread::~HostSocketThread()
{
    this->_thread->exit();
    this->_thread->deleteLater();
    this->_thread = nullptr;
}

void HostSocketThread::onReceiveMessage()
{
    QIODevice* socket = dynamic_cast<QIODevice*>(this->sender());
    auto msg = Message::parse(*socket);
    if (msg != nullptr) {
        this->_host->_onReceiveMessage(msg, this);
    }
}

void HostSocketThread::onUnregister()
{
    this->_host->_unregister(this, true);
    delete this;
}

qint64 HostSocketThread::write(const QByteArray &data)
{
    return this->_socket->write(data);
}

void HostSocketThread::closeAndDelete()
{
    this->_socket->close();
    this->_socket->deleteLater();
    this->_socket = nullptr;
    delete this;
}

#include "moc_hostsocketthread.cpp"

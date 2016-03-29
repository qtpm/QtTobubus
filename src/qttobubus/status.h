#pragma once

#include <QObject>

class Call : public QObject {
    Q_OBJECT

    Q_ENUMS(Status)

    explicit Call(QObject *parent = 0);
    virtual ~Call() = default;
public:
    static void RegisterStatus();
    enum Status {Success, SocketClosed, InvalidPath, InvalidMethod, RuntimeError, SerializeError, CommunicationError, DeserializeError, RemoteError};
};

Q_DECLARE_METATYPE(Call::Status)

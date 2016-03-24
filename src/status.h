#ifndef STATUS_H
#define STATUS_H

#include <QObject>

class Call : public QObject {
    Q_OBJECT

    Q_ENUMS(Status)

    explicit Call(QObject *parent = 0);
    virtual ~Call() = default;
public:
    enum Status {Success, SocketClosed, InvalidPath, InvalidMethod, SerializeError, CommunicationError, DeserializeError, RemoteError};
};

#endif // STATUS_H


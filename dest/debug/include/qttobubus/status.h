#ifndef STATUS_H
#define STATUS_H

#include "qttobubus_global.h"
#include <QObject>

class QTTOBUBUSSHARED_EXPORT Call : public QObject {
    Q_OBJECT

    Q_ENUMS(Status)

    explicit Call(QObject *parent = 0);
    virtual ~Call() = default;
public:
    enum Status {Success, SocketClosed, InvalidPath, InvalidMethod, SerializeError, CommunicationError, DeserializeError, RemoteError};
};

#endif // STATUS_H


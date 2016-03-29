#pragma once

#include "message.h"
#include <QMap>
#include <QMutex>

enum SessionStrategy { RecycleStrategy, IncrementStrategy };

struct Session
{
    Message* message;
    std::function<void(Message*)> callback;
};

class SessionManager
{
	QMutex _mutex;
    QMap<quint32, Session> _sessions;

	SessionStrategy _strategy;
	quint32 _nextSessionID;

public:
	explicit SessionManager(SessionStrategy strategy = RecycleStrategy);
	quint32 getUniqueSessionID();
    void receive(quint32 sessionID, std::function<void(Message*)> callback);
    void receiveAndClose(quint32 sessionID);
    void close(quint32 sessionID);
    void send(quint32 sessionID, Message* msg);
};

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "chan.h"
#include "message.h"
#include <QMap>
#include <QMutex>

enum SessionStrategy { RecycleStrategy, IncrementStrategy };

class SessionManager
{
	QMutex _mutex;
	QMap<quint32, channel::chan<Message*>*> _sessions;
	SessionStrategy _strategy;
	quint32 _nextSessionID;

public:
	explicit SessionManager(SessionStrategy strategy = RecycleStrategy);
	quint32 getUniqueSessionID();
	Message* receiveAndClose(quint32 sessionID);
	channel::chan<Message*>* getChannelOfSessionID(quint32);
};

#endif // SESSIONMANAGER_H

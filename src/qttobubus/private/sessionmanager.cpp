#include "sessionmanager.h"
#include <QMutexLocker>

SessionManager::SessionManager(SessionStrategy strategy)
	: _strategy(strategy)
	, _nextSessionID(0)
{
}

quint32 SessionManager::getUniqueSessionID()
{
	QMutexLocker locker(&this->_mutex);
	if (this->_strategy == RecycleStrategy) {
		quint32 id;
		for (id = 0; id < 4294967295; id++) {
			if (!this->_sessions.contains(id)) {
                this->_sessions[id] = Session{};
				return id;
			}
		}
	} else {
		return this->_nextSessionID++;
	}
	Q_ASSERT(false);
    return 0;
}

void SessionManager::receive(quint32 sessionID, std::function<void(Message*)> callback)
{
    this->_mutex.lock();
    if (this->_sessions.contains(sessionID)) {
        if (this->_sessions[sessionID].message) {
            auto session = this->_sessions.take(sessionID);
            this->_mutex.unlock();
            callback(session.message);
        } else {
            this->_sessions[sessionID].callback = callback;
            this->_mutex.unlock();
        }
    } else {
        this->_sessions[sessionID] = Session{nullptr, callback};
        this->_mutex.unlock();
    }
}

void SessionManager::receiveAndClose(quint32 sessionID)
{
    this->receive(sessionID, [](Message* message) {
        // do nothing;
    });
}

void SessionManager::close(quint32 sessionID)
{
    QMutexLocker locker(&this->_mutex);
    this->_sessions.remove(sessionID);
}

void SessionManager::send(quint32 sessionID, Message *msg)
{
    this->_mutex.lock();
    if (this->_sessions.contains(sessionID)) {
        if (this->_sessions[sessionID].callback) {
            auto session = this->_sessions.take(sessionID);
            this->_mutex.unlock();
            session.callback(msg);
        } else {
            this->_sessions[sessionID].message = msg;
            this->_mutex.unlock();
        }
    } else {
        this->_sessions[sessionID] = Session{msg, nullptr};
        this->_mutex.unlock();
    }
}

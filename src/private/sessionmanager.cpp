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
				this->_sessions[id] = new channel::chan<Message*>();
				return id;
			}
		}
	} else {
		return this->_nextSessionID++;
	}
	Q_ASSERT(false);
	return 0;
}

Message* SessionManager::receiveAndClose(quint32 sessionID)
{
	this->_mutex.lock();
	channel::chan<Message*>* chan;
	if (!this->_sessions.contains(sessionID)) {
		chan = new channel::chan<Message*>();
		this->_sessions[sessionID] = chan;
	} else {
		chan = this->_sessions[sessionID];
	}
	this->_mutex.unlock();
    Message* result = nullptr;
	chan->recv(result);
	this->_sessions.remove(sessionID);
	return result;
}

channel::chan<Message*>* SessionManager::getChannelOfSessionID(quint32 sessionID)
{
	QMutexLocker locker(&this->_mutex);
	auto iter = this->_sessions.find(sessionID);
	if (iter != this->_sessions.end()) {
		return iter.value();
	}
	auto chan = new channel::chan<Message*>();
	this->_sessions[sessionID] = chan;
	return chan;
}

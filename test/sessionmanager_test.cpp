#include "private/sessionmanager.h"
#include <QDebug>
#include <QString>
#include <QtConcurrent>
#include <QtTest>

class TestSessionManager : public QObject
{
	Q_OBJECT

public:
	TestSessionManager() {}

private Q_SLOTS:
	void testGetUniqueSessionWithRecycleStrategy()
	{
		SessionManager manager;
		QCOMPARE(manager.getUniqueSessionID(), (quint32)0);
		QCOMPARE(manager.getUniqueSessionID(), (quint32)1);
		QCOMPARE(manager.getUniqueSessionID(), (quint32)2);
		auto future = QtConcurrent::run([&manager] {
			auto channel = manager.getChannelOfSessionID(1);
			channel->send(nullptr);
		});
		manager.receiveAndClose(1);
		QCOMPARE(manager.getUniqueSessionID(), (quint32)1);
	}

	void testGetUniqueSessionWithIncrementStrategy()
	{
		SessionManager manager(IncrementStrategy);
		QCOMPARE(manager.getUniqueSessionID(), (quint32)0);
		QCOMPARE(manager.getUniqueSessionID(), (quint32)1);
		QCOMPARE(manager.getUniqueSessionID(), (quint32)2);
		auto future = QtConcurrent::run([&manager] {
			auto channel = manager.getChannelOfSessionID(1);
			channel->send(nullptr);
		});
		manager.receiveAndClose(1);
		QCOMPARE(manager.getUniqueSessionID(), (quint32)3);
	}
};

QTEST_APPLESS_MAIN(TestSessionManager)

#include "sessionmanager_test.moc"

#include "qttobubus/private/proxy.h"
#include "sample.h"
#include <QString>
#include <QtTest>

class TestProxy : public QObject
{
	Q_OBJECT

public:
	TestProxy() {}

private Q_SLOTS:
	void testProxyRegister()
	{
		auto proxy = new Proxy(QSharedPointer<QObject>(new Sample()));
		QVERIFY(proxy->hasMethod("testFunction"));
		QVERIFY(!proxy->hasMethod("noExistFunction"));

		auto result = proxy->call("testFunction", QVariantList{10});
		QCOMPARE(result.toInt(), 100);

		auto result2 = proxy->call("testFunction", QVariantList{20});
		QCOMPARE(result2.toInt(), 200);

		auto result3 = proxy->call("testFunction2", QVariantList{"name", QVariantList{6.5, true}});
		QCOMPARE(result3.toString(), QString("called"));
	}
};

QTEST_APPLESS_MAIN(TestProxy)

#include "proxy_test.moc"

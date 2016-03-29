#include "qttobubus/private/message.h"
#include <QBuffer>
#include <QByteArray>
#include <QScopedPointer>
#include <QString>
#include <QtTest>

class TestMessage : public QObject
{
	Q_OBJECT

public:
	TestMessage() {}

private Q_SLOTS:
	void testArchiveMessage()
	{
		auto expected = QByteArray("\x10\x00\x00\x00\x00\x00\x00\x00\x1e\x00\x00"
								   "\x00github.com/shibukawa/tobubus/1",
			30 + 12);
		auto actual = Message::archive(ConnectClient, 0, "github.com/shibukawa/tobubus/1");
		QCOMPARE(actual, expected);
	}

	void testParseMessage()
	{
		auto data = QByteArray("\x10\x00\x00\x00\x00\x00\x00\x00\x1e\x00\x00\x00git"
							   "hub.com/shibukawa/tobubus/1",
			30 + 12);
		auto expectedBody = QByteArray("github.com/shibukawa/tobubus/1", 30);

		QBuffer buffer(&data);
		buffer.open(QBuffer::ReadOnly);
		QScopedPointer<Message> message(Message::parse(buffer));

		QCOMPARE(message->type, ConnectClient);
		QCOMPARE(message->body, expectedBody);
	}

	void testArchiveMethodCall()
	{
		auto data = MethodCall::archive(
			ConnectClient, 0, "/image", "open", QVariantList{"test.png", 0777});

		QBuffer buffer(&data);
		buffer.open(QBuffer::ReadOnly);
		auto message = Message::parse(buffer);
		QScopedPointer<MethodCall> methodCall(MethodCall::parse(message->body));

		QCOMPARE(methodCall->path, QString("/image"));
		QCOMPARE(methodCall->method, QString("open"));
		QCOMPARE(methodCall->params[0].toString(), QString("test.png"));
		QCOMPARE(methodCall->params[1].toInt(), 0777);
	}
};

QTEST_APPLESS_MAIN(TestMessage)

#include "message_test.moc"

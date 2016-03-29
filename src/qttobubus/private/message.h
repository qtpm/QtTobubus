#pragma once

#include <QByteArray>
#include <QLocalSocket>
#include <QVariantList>
#include <cstdint>

enum MessageType {
	InvalidMessage = 0,
	ResultOK = 0x1,
	ResultNG = 0x2,
	ResultObjectNotFound = 0x3,
	ResultMethodNotFound = 0x4,
    ResultMethodError = 0x5,
    ConnectClient = 0x10,
	CloseClient = 0x11,
	ConfirmPath = 0x20,
	Publish = 0x21,
	Unpublish = 0x22,
	CallMethod = 0x30,
	ReturnMethod = 0x31
};

struct Message {
	MessageType type;
	quint32 id;
	QByteArray body;

	static Message* parse(QIODevice& socket);
	static QByteArray archive(MessageType type, quint32 sessionID);
	static QByteArray archive(MessageType type, quint32 sessionID, const QByteArray& body);
};

struct MethodCall {
	QString path;
	QString method;
	QList<QVariant> params;

	static MethodCall* parse(const QByteArray& data);
	static QByteArray archive(MessageType type, quint32 sessionID, const QString& path,
		const QString& methodName, const QVariantList& params);
};

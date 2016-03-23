#include "message.h"
#include <QDataStream>
#include <QMap>
#include <qtcbor/qtcbor.h>

Message* Message::parse(QIODevice& socket)
{
	QDataStream stream(&socket);
	stream.setByteOrder(QDataStream::LittleEndian);
	quint32 type, sessionID, size;
	stream >> type >> sessionID >> size;
	auto result = new Message{(MessageType)type, sessionID};
	result->body.resize(size);
	auto readSize = stream.readRawData(result->body.data(), size);
	if (readSize != size) {
		delete result;
		return nullptr;
	}
	return result;
}

QByteArray Message::archive(MessageType type, quint32 sessionID)
{
	QByteArray result;
	QDataStream stream(&result, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream << (quint32)type << sessionID << (quint32)0;
    return result;
}

QByteArray Message::archive(MessageType type, quint32 sessionID, const QByteArray& body)
{
	QByteArray result;
	QDataStream stream(&result, QIODevice::WriteOnly);
	stream.setByteOrder(QDataStream::LittleEndian);
	stream << (quint32)type << sessionID << (quint32)body.size();
	stream.writeRawData(body.constData(), body.size());
    return result;
}

MethodCall* MethodCall::parse(const QByteArray& data)
{
	auto map = QCBOR::unpack(data).toMap();
	auto result
		= new MethodCall{map["path"].toString(), map["method"].toString(), map["params"].toList()};
	return result;
}

QByteArray MethodCall::archive(MessageType type, quint32 sessionID, const QString& path,
	const QString& methodName, const QVariantList& params)
{
	QMap<QString, QVariant> map{{"path", path}, {"method", methodName}, {"params", params}};
    return Message::archive(type, sessionID, QCBOR::pack(map));
}

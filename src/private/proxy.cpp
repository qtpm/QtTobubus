#include "proxy.h"
#include <QDebug>
#include <QMetaMethod>

enum ArgType {
	IntType,
	FloatType,
	BoolType,
	StringType,
	StringListType,
	ListType,
	MapType,
	VariantType,
	NullType
};

struct Method {
	QMetaMethod method;
	ArgType returnType;
	QVector<ArgType> paramTypes;
};

static QMap<QByteArray, ArgType> types{
	{"void", NullType}, {"int", IntType}, {"uint", IntType}, {"short", IntType},
	{"ushort", IntType}, {"long", IntType}, {"ulong", IntType}, {"qlong", IntType},
	{"qulong", IntType}, {"qlonglong", IntType}, {"qulonglong", IntType}, {"float", FloatType},
	{"double", FloatType}, {"bool", BoolType}, {"QString", StringType},
	{"QStringList", StringListType}, {"QVariant", VariantType}, {"QVariantList", ListType},
	{"QVariantMap", MapType},
};

Proxy::Proxy(QSharedPointer<QObject> instance)
{
	this->_instance = instance;
	auto meta = instance->metaObject();
	for (int i = meta->methodOffset(); i < meta->methodCount(); ++i) {
		auto metaMethod = meta->method(i);
		QString name(metaMethod.name());
		if (!types.contains(metaMethod.typeName())) {
			qCritical() << "Proxy::Proxy(): return type of " << meta->className() << "'s " << name
						<< " (" << metaMethod.typeName() << ") is not supported";
			Q_ASSERT(false);
		}
		auto method = new Method{metaMethod, types[metaMethod.typeName()]};
		for (auto& t : metaMethod.parameterTypes()) {
			if (!types.contains(t) || t == "void") {
				qCritical() << "Proxy::Proxy(): argument type of " << meta->className() << "'s "
							<< name << " (" << t << ") is not supported";
				Q_ASSERT(false);
			}
			method->paramTypes.append(types[t]);
		}
		this->_methods[name] = method;
	}
}

Proxy::~Proxy()
{
	for (auto& method : this->_methods) {
		delete method;
	}
}

QVariant Proxy::call(const QString& name, QVariantList params)
{
	auto iter = this->_methods.find(name);

	if (this->_methods.end() != iter) {
		auto method = iter.value();
        int returnInt = 0;
        double returnFlaot = 0.0;
        bool returnBool = false;
		QString returnString;
		QStringList returnStringList;
		QVariantList returnList;
		QVariantMap returnMap;
		QVariant returnVariant;

		QGenericReturnArgument returnArgument;

		switch (method->returnType) {
			case IntType:
				returnArgument = Q_RETURN_ARG(int, returnInt);
				break;
			case FloatType:
				returnArgument = Q_RETURN_ARG(double, returnFlaot);
				break;
			case BoolType:
				returnArgument = Q_RETURN_ARG(bool, returnBool);
				break;
			case StringType:
				returnArgument = Q_RETURN_ARG(QString, returnString);
				break;
			case StringListType:
				returnArgument = Q_RETURN_ARG(QStringList, returnStringList);
				break;
			case ListType:
				returnArgument = Q_RETURN_ARG(QVariantList, returnList);
				break;
			case MapType:
				returnArgument = Q_RETURN_ARG(QVariantMap, returnMap);
				break;
			case VariantType:
				returnArgument = Q_RETURN_ARG(QVariant, returnVariant);
				break;
			default:
				break;
		}
		QList<QGenericArgument> arguments;
		for (int i = 0; i < params.size(); i++) {
			switch (method->paramTypes[i]) {
				case IntType:
					arguments.append(Q_ARG(int, params[i].toInt()));
					break;
				case FloatType:
					arguments.append(Q_ARG(double, params[i].toDouble()));
					break;
				case BoolType:
					arguments.append(Q_ARG(bool, params[i].toBool()));
					break;
				case StringType:
					arguments.append(Q_ARG(QString, params[i].toString()));
					break;
				case StringListType:
					arguments.append(Q_ARG(QStringList, params[i].toStringList()));
					break;
				case ListType:
					arguments.append(Q_ARG(QVariantList, params[i].toList()));
					break;
				case MapType:
					arguments.append(Q_ARG(QVariantMap, params[i].toMap()));
					break;
				case VariantType:
					arguments.append(Q_ARG(QVariant, params[i]));
					break;
				default:
					Q_ASSERT(false); // void is not supported as a paramter
					break;
			}
		}
        auto ok = false;
		switch (params.size()) {
			case 0:
				ok = method->method.invoke(this->_instance.data(), returnArgument);
				break;
			case 1:
				ok = method->method.invoke(this->_instance.data(), returnArgument, arguments[0]);
				break;
			case 2:
				ok = method->method.invoke(
					this->_instance.data(), returnArgument, arguments[0], arguments[1]);
				break;
			case 3:
				ok = method->method.invoke(this->_instance.data(), returnArgument, arguments[0],
					arguments[1], arguments[2]);
				break;
			case 4:
				ok = method->method.invoke(this->_instance.data(), returnArgument, arguments[0],
					arguments[1], arguments[2], arguments[3]);
				break;
			case 5:
				ok = method->method.invoke(this->_instance.data(), returnArgument, arguments[0],
					arguments[1], arguments[2], arguments[3], arguments[4]);
				break;
			case 6:
				ok = method->method.invoke(this->_instance.data(), returnArgument, arguments[0],
					arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
				break;
			case 7:
				ok = method->method.invoke(this->_instance.data(), returnArgument, arguments[0],
					arguments[1], arguments[2], arguments[3], arguments[4], arguments[5],
					arguments[6]);
				break;
			case 8:
				ok = method->method.invoke(this->_instance.data(), returnArgument, arguments[0],
					arguments[1], arguments[2], arguments[3], arguments[4], arguments[5],
					arguments[6], arguments[7]);
				break;
			case 9:
				ok = method->method.invoke(this->_instance.data(), returnArgument, arguments[0],
					arguments[1], arguments[2], arguments[3], arguments[4], arguments[5],
					arguments[6], arguments[7], arguments[8]);
				break;
			case 10:
				ok = method->method.invoke(this->_instance.data(), returnArgument, arguments[0],
					arguments[1], arguments[2], arguments[3], arguments[4], arguments[5],
					arguments[6], arguments[7], arguments[8], arguments[9]);
				break;
		}

		if (!ok) {
			return QVariant();
		}
		switch (method->returnType) {
			case IntType:
				return QVariant(returnInt);
			case FloatType:
				return QVariant(returnFlaot);
			case BoolType:
				return QVariant(returnBool);
			case StringType:
				return QVariant(returnString);
			case StringListType:
				return QVariant(returnStringList);
			case ListType:
				return QVariant(returnList);
			case MapType:
				return QVariant(returnMap);
			case VariantType:
				return QVariant(returnVariant);
			default:
				break;
		}
	}
	return QVariant();
}

bool Proxy::hasMethod(const QString& name) const { return this->_methods.contains(name); }

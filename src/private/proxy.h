#ifndef PROXY_H
#define PROXY_H

#include <QObject>
#include <QSet>
#include <QSharedPointer>
#include <QVariant>

struct Method;

class Proxy
{
	QSharedPointer<QObject> _instance;
	QMap<QString, Method*> _methods;

public:
	explicit Proxy(QSharedPointer<QObject> instance);
	virtual ~Proxy();
	QVariant call(const QString& name, QVariantList args);
	bool hasMethod(const QString& name) const;
};

#endif // PROXY_H

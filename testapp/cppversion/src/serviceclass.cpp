#include "serviceclass.h"

ServiceClass::ServiceClass(QObject* parent)
	: QObject(parent)
{
}

int ServiceClass::Fib(int i)
{
	if (i < 2) {
		return i;
	}
	return Fib(i - 1) + Fib(i - 2);
}

QString ServiceClass::Greeting(QString name) { return QString("Hello ") + name; }

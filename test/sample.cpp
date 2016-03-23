#include "sample.h"
#include <QDebug>

Sample::Sample(QObject* parent)
	: QObject(parent)
{
}

int Sample::testFunction(int input) { return input * 10; }

QString Sample::testFunction2(QString name, QVariantList list)
{
	qDebug() << name;
	qDebug() << list;
	return "called";
}

#include "moc_sample.cpp"

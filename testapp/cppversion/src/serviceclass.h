#ifndef SERVICECLASS_H
#define SERVICECLASS_H

#include <QObject>

class ServiceClass : public QObject
{
    Q_OBJECT
public:
    explicit ServiceClass(QObject *parent = 0);

signals:

public slots:
    int Fib(int i);
    QString Greeting(QString name);
};

#endif // SERVICECLASS_H

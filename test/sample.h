#ifndef SAMPLE_H
#define SAMPLE_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QVariant>

class Sample : public QObject {
    Q_OBJECT
public:
    explicit Sample(QObject *parent = 0);
signals:
public slots:
    int testFunction(int input);
    QString testFunction2(QString name, QVariantList list);
};

#endif // SAMPLE_H


#ifndef QTOBUBUSPLUGIN_H
#define QTOBUBUSPLUGIN_H

#include "qttobubus_global.h"
#include <QObject>

class QTTOBUBUSSHARED_EXPORT QTobubusPlugin : public QObject {
    Q_OBJECT
public:
    explicit QTobubusPlugin(QObject *parent = 0);
signals:
public slots:
};

#endif // QTOBUBUSPLUGIN_H


#ifndef RESPONSE_H
#define RESPONSE_H

#include <QObject>

class Response : public QObject {
    Q_OBJECT
public:
    explicit Response(QObject *parent = 0);
    virtual ~Response() = default;
signals:
public slots:
};

#endif // RESPONSE_H


#pragma once

#include <QVariant>
#include "aliasmethod.h"
#include "status.h"

class QTobubusHost;

template <typename T>
class AliasObject {
private:
    T * _dispatcher;
    QString _path ;
public:
    AliasObject(T * _dispatcher = nullptr, const QString& path = "")  : _dispatcher(_dispatcher), _path(path) {
    }

    AliasMethod<T> operator[](const QString& method) {
        return std::move(AliasMethod<T>(this->_dispatcher, this->_path, method));
    }

    QVariant apply(const QString& method, QVariantList params) {
        return _dispatcher->apply(this->_path, method, params);
    }

    QVariant apply(const QString& method, Call::Status *status, QVariantList params) {
        return _dispatcher->apply(this->_path, method, status, params);
    }
};

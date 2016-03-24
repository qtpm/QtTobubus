#ifndef ALIASMETHOD_H
#define ALIASMETHOD_H

#include "status.h"

template <typename T>
class AliasMethod {
private:
    T* _dispatcher;
    QString _path ;
    QString _method;

public:
    AliasMethod(T * dispacher = nullptr, const QString& path = "", const QString& method = "")
        : _dispatcher(dispacher), _path(path), _method(method) {
    }

    QVariant operator()() {
        return this->apply(QVariantList{});
    }

    QVariant operator()(QVariant arg1) {
        return this->apply(QVariantList{arg1});
    }

    QVariant operator()(QVariant arg1, QVariant arg2) {

        return this->apply(QVariantList{arg1, arg2});
    }

    QVariant operator()(QVariant arg1, QVariant arg2, QVariant arg3){
        return this->apply(QVariantList{arg1, arg2, arg3});

    }

    QVariant operator()(QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4){
        return this->apply(QVariantList{arg1, arg2, arg3, arg4});

    }

    QVariant operator()(QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5){
        return this->apply(QVariantList{arg1, arg2, arg3, arg4, arg5});

    }

    QVariant operator()(QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6){
        return this->apply(QVariantList{arg1, arg2, arg3, arg4, arg5, arg6});

    }

    QVariant operator()(QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7){
        return this->apply(QVariantList{arg1, arg2, arg3, arg4, arg5, arg6, arg7});

    }

    QVariant operator()(QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8){
        return this->apply(QVariantList{arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8});

    }

    QVariant operator()(QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9){
        return this->apply(QVariantList{arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9});

    }

    QVariant operator()(QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5, QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9, QVariant arg10){
        return this->apply(QVariantList{arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10});

    }

    QVariant operator()(Call::Status *status){
        return this->apply(status, QVariantList{});

    }

    QVariant operator()(Call::Status *status, QVariant arg1){

       return this->apply(status, QVariantList{arg1});

    }

    QVariant operator()(Call::Status *status, QVariant arg1, QVariant arg2){
       return this->apply(status, QVariantList{arg1, arg2});

    }

    QVariant operator()(Call::Status *status, QVariant arg1, QVariant arg2, QVariant arg3){
       return this->apply(status, QVariantList{arg1, arg2, arg3});

    }

    QVariant operator()(Call::Status *status, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4){
        return this->apply(status, QVariantList{arg1, arg2, arg3, arg4});

    }

    QVariant operator()(Call::Status *status, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5){
        return this->apply(status,  QVariantList{arg1, arg2, arg3, arg4, arg5});

    }

    QVariant operator()(Call::Status *status, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5,
                                       QVariant arg6){
        return this->apply(status, QVariantList{arg1, arg2, arg3, arg4, arg5, arg6});

    }

    QVariant operator()(Call::Status *status, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5,
                                       QVariant arg6, QVariant arg7){
       return this->apply(status,  QVariantList{arg1, arg2, arg3, arg4, arg5, arg6, arg7});

    }

    QVariant operator()(Call::Status *status, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5,
                                       QVariant arg6, QVariant arg7, QVariant arg8){
        return this->apply(status, QVariantList{arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8});

    }

    QVariant operator()(Call::Status *status, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5,
                                       QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9){
        return this->apply(status, QVariantList{arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9});

    }

    QVariant operator()(Call::Status *status, QVariant arg1, QVariant arg2, QVariant arg3, QVariant arg4, QVariant arg5,
                                       QVariant arg6, QVariant arg7, QVariant arg8, QVariant arg9, QVariant arg10){
      return this->apply(status, QVariantList{arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10});

    }

    QVariant apply(QVariantList params) {
        return this->_dispatcher->apply(this->_path, this->_method, params);
    }

    QVariant apply(Call::Status *status, QVariantList params) {
        return this->_dispatcher->apply(this->_path, this->_method, params);
    }
};

#endif // ALIASMETHOD_H


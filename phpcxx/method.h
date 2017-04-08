#ifndef PHPCXX_METHOD_H
#define PHPCXX_METHOD_H

#include "phpcxx.h"
#include "function.h"

namespace phpcxx {

class PHPCXX_EXPORT Method : public Function {
public:
    using Function::Function;

    enum Access {
        Public    = ZEND_ACC_PUBLIC,
        Protected = ZEND_ACC_PROTECTED,
        Private   = ZEND_ACC_PRIVATE
    };

    Method&& setAccess(Method::Access access);
    Method&& setStatic(bool v);
    Method&& setAbstract(bool v);
    Method&& setFinal(bool v);
};

template<typename T, MethodPrototypeNN<T> Handler>
static inline Method createMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, required, optional, byRef);
}

template<typename T, ConstMethodPrototypeNN<T> Handler>
static inline Method createMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, required, optional, byRef);
}

template<typename T, MethodPrototypeNV<T> Handler>
static inline Method createMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, required, optional, byRef);
}

template<typename T, ConstMethodPrototypeNV<T> Handler>
static inline Method createMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, required, optional, byRef);
}

template<typename T, MethodPrototypeVN<T> Handler>
static inline Method createMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, required, optional, byRef);
}

template<typename T, ConstMethodPrototypeVN<T> Handler>
static inline Method createMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, required, optional, byRef);
}

template<typename T, MethodPrototypeVV<T> Handler>
static inline Method createMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, required, optional, byRef);
}

template<typename T, ConstMethodPrototypeVV<T> Handler>
static inline Method createMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, required, optional, byRef);
}

}

#endif /* PHPCXX_METHOD_H */

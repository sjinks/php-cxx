#ifndef PHPCXX_METHOD_H
#define PHPCXX_METHOD_H

#include "phpcxx.h"
#include "function.h"

namespace phpcxx {

class PHPCXX_EXPORT Method : public Function {
public:
    [[gnu::nonnull]] Method(const char* name, InternalFunction c, std::size_t nreq = 0, const Arguments& args = {}, bool byref = false);
    Method(const Method& other);

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
static inline Method createMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, nreq, args, byRef);
}

template<typename T, ConstMethodPrototypeNN<T> Handler>
static inline Method createMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, nreq, args, byRef);
}

template<typename T, MethodPrototypeNV<T> Handler>
static inline Method createMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, nreq, args, byRef);
}

template<typename T, ConstMethodPrototypeNV<T> Handler>
static inline Method createMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, nreq, args, byRef);
}

template<typename T, MethodPrototypeVN<T> Handler>
static inline Method createMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, nreq, args, byRef);
}

template<typename T, ConstMethodPrototypeVN<T> Handler>
static inline Method createMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, nreq, args, byRef);
}

template<typename T, MethodPrototypeVV<T> Handler>
static inline Method createMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, nreq, args, byRef);
}

template<typename T, ConstMethodPrototypeVV<T> Handler>
static inline Method createMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<T, Handler>, nreq, args, byRef);
}

template<FunctionPrototypeNN Handler>
static inline Method createStaticMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<Handler>, nreq, args, byRef).setStatic(true);
}

template<FunctionPrototypeNV Handler>
static inline Method createStaticMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<Handler>, nreq, args, byRef).setStatic(true);
}

template<FunctionPrototypeVN Handler>
static inline Method createStaticMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<Handler>, nreq, args, byRef).setStatic(true);
}

template<FunctionPrototypeVV Handler>
static inline Method createStaticMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Method(name, &FunctionHandler::handler<Handler>, nreq, args, byRef).setStatic(true);
}

}

#endif /* PHPCXX_METHOD_H */

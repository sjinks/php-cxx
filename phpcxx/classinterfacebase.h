#ifndef CLASSINTERFACEBASE_H
#define CLASSINTERFACEBASE_H

#include "phpcxx.h"
#include "classbase.h"
#include "emallocallocator.h"
#include "function.h"
#include "functionhandler.h"

namespace phpcxx {

template<typename T>
class PHPCXX_EXPORT ClassInterfaceBase : public ClassBase {
public:
    using ClassBase::ClassBase;

protected:
    virtual phpcxx::ClassImplementationBase* construct()
    {
        return phpcxx::emcreate<T>();
    }

    template<MethodPrototypeNN<T> Handler>
    phpcxx::Method addMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
    {
        return phpcxx::createMethod<T, Handler>(name, required, optional, byRef);
    }

    template<MethodPrototypeNV<T> Handler>
    phpcxx::Method addMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
    {
        return phpcxx::createMethod<T, Handler>(name, required, optional, byRef);
    }

    template<MethodPrototypeVN<T> Handler>
    phpcxx::Method addMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
    {
        return phpcxx::createMethod<T, Handler>(name, required, optional, byRef);
    }

    template<MethodPrototypeVV<T> Handler>
    phpcxx::Method addMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
    {
        return phpcxx::createMethod<T, Handler>(name, required, optional, byRef);
    }

    template<ConstMethodPrototypeNN<T> Handler>
    phpcxx::Method addMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
    {
        return phpcxx::createMethod<T, Handler>(name, required, optional, byRef);
    }

    template<ConstMethodPrototypeNV<T> Handler>
    phpcxx::Method addMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
    {
        return phpcxx::createMethod<T, Handler>(name, required, optional, byRef);
    }

    template<ConstMethodPrototypeVN<T> Handler>
    phpcxx::Method addMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
    {
        return phpcxx::createMethod<T, Handler>(name, required, optional, byRef);
    }

    template<ConstMethodPrototypeVV<T> Handler>
    phpcxx::Method addMethod(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
    {
        return phpcxx::createMethod<T, Handler>(name, required, optional, byRef);
    }
};

}

#endif /* CLASSINTERFACEBASE_H */

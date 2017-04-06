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
    phpcxx::Function addMethod(const char* name)
    {
        return phpcxx::createMethod<T, Handler>(name);
    }

    template<MethodPrototypeNV<T> Handler>
    phpcxx::Function addMethod(const char* name)
    {
        return phpcxx::createMethod<T, Handler>(name);
    }

    template<MethodPrototypeVN<T> Handler>
    phpcxx::Function addMethod(const char* name)
    {
        return phpcxx::createMethod<T, Handler>(name);
    }

    template<MethodPrototypeVV<T> Handler>
    phpcxx::Function addMethod(const char* name)
    {
        return phpcxx::createMethod<T, Handler>(name);
    }

    template<ConstMethodPrototypeNN<T> Handler>
    phpcxx::Function addMethod(const char* name)
    {
        return phpcxx::createMethod<T, Handler>(name);
    }

    template<ConstMethodPrototypeNV<T> Handler>
    phpcxx::Function addMethod(const char* name)
    {
        return phpcxx::createMethod<T, Handler>(name);
    }

    template<ConstMethodPrototypeVN<T> Handler>
    phpcxx::Function addMethod(const char* name)
    {
        return phpcxx::createMethod<T, Handler>(name);
    }

    template<ConstMethodPrototypeVV<T> Handler>
    phpcxx::Function addMethod(const char* name)
    {
        return phpcxx::createMethod<T, Handler>(name);
    }
};

}

#endif /* CLASSINTERFACEBASE_H */

#ifndef CLASSINTERFACEBASE_H
#define CLASSINTERFACEBASE_H

#include "phpcxx.h"
#include "classbase.h"
#include "emallocallocator.h"
#include "function.h"
#include "functionhandler.h"

namespace phpcxx {

template<typename T>
class PHPCXX_EXPORT PhpClass final : public ClassBase {
public:
    using ClassBase::ClassBase;

    template<MethodPrototypeNN<T> Handler>
    phpcxx::Method addMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createMethod<T, Handler>(name, nreq, args, byRef));
    }

    template<MethodPrototypeNV<T> Handler>
    phpcxx::Method addMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createMethod<T, Handler>(name, nreq, args, byRef));
    }

    template<MethodPrototypeVN<T> Handler>
    phpcxx::Method addMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createMethod<T, Handler>(name, nreq, args, byRef));
    }

    template<MethodPrototypeVV<T> Handler>
    phpcxx::Method addMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createMethod<T, Handler>(name, nreq, args, byRef));
    }

    template<ConstMethodPrototypeNN<T> Handler>
    phpcxx::Method addMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createMethod<T, Handler>(name, nreq, args, byRef));
    }

    template<ConstMethodPrototypeNV<T> Handler>
    phpcxx::Method addMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createMethod<T, Handler>(name, nreq, args, byRef));
    }

    template<ConstMethodPrototypeVN<T> Handler>
    phpcxx::Method addMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createMethod<T, Handler>(name, nreq, args, byRef));
    }

    template<ConstMethodPrototypeVV<T> Handler>
    phpcxx::Method addMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createMethod<T, Handler>(name, nreq, args, byRef));
    }

    template<FunctionPrototypeNN Handler>
    phpcxx::Method addStaticMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createStaticMethod<Handler>(name, nreq, args, byRef));
    }

    template<FunctionPrototypeNV Handler>
    phpcxx::Method addStaticMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createStaticMethod<Handler>(name, nreq, args, byRef));
    }

    template<FunctionPrototypeVN Handler>
    phpcxx::Method addStaticMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createStaticMethod<Handler>(name, nreq, args, byRef));
    }

    template<FunctionPrototypeVV Handler>
    phpcxx::Method addStaticMethod(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
    {
        return this->addClassMethod(phpcxx::createStaticMethod<Handler>(name, nreq, args, byRef));
    }

protected:
    virtual phpcxx::ClassImplementationBase* construct()
    {
        return phpcxx::emcreate<T>();
    }
};

}

#endif /* CLASSINTERFACEBASE_H */

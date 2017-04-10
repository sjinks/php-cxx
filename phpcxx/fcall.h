#ifndef PHPCXX_FCALL_H
#define PHPCXX_FCALL_H

#include "phpcxx.h"
#include "vector.h"
#include "helpers.h"
#include "callable.h"

extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_API.h>
}

namespace phpcxx {

class Value;

class PHPCXX_EXPORT FCall {
public:
    FCall(const zend_fcall_info& fci, const zend_fcall_info_cache& fcc);
    FCall(const Callable& c);

    [[gnu::nonnull]] FCall(const char* name)
        : FCall(Callable(name))
    {
    }

    [[gnu::nonnull]] FCall(const char* cls, const char* name)
        : FCall(Callable(cls, name))
    {
    }

    [[gnu::nonnull]] FCall(zval* obj, const char* name)
        : FCall(Callable(name))
    {
    }

    [[gnu::nonnull]] FCall(zval* arg)
        : FCall(Callable(arg))
    {
    }

    Value operator()(vector<Value> args);
    Value operator()(vector<zval> args);

    template<typename... Params>
    Value operator()(Params&&... p);

private:
    zend_fcall_info m_fci;
    zend_fcall_info_cache m_fcc;

    Value operator()(std::size_t n, zval* args);

    template<typename T>
    static zval* paramHelper(T&& v, zval& z) { construct_zval(z, v); Z_TRY_DELREF(z); return &z; }

    template<typename ...Args, std::size_t ...Is>
    Value call(indices<Is...>, Args&&... args);

};

template<typename... Params> phpcxx::Value call(const char* name, Params&&... p);
template<typename... Params> phpcxx::Value call(const phpcxx::Value& v, Params&&... p);

}

#include "fcall.tcc"

#endif /* PHPCXX_FCALL_H */

#ifndef PHPCXX_FCALL_TCC
#define PHPCXX_FCALL_TCC

#ifndef PHPCXX_FCALL_H
#error "Please do not include this file directly, use fcall.h instead"
#endif

#include "array.h"
#include "value.h"

namespace phpcxx {

template<typename... Params>
inline Value call(const char* name, Params&&... p)
{
    return FCall(Callable(name))(std::forward<Params>(p)...);
}

template<typename... Params>
inline Value call(const Value& v, Params&&... p)
{
    return FCall(Callable(v.pzval()))(std::forward<Params>(p)...);
}

template<> inline zval* FCall::paramHelper(phpcxx::Value&& v, zval&) { return v.pzval(); }
template<> inline zval* FCall::paramHelper(phpcxx::Array&& v, zval&) { return v.pzval(); }

template<typename... Params>
inline phpcxx::Value FCall::operator()(Params&&... p)
{
    return this->call(IndicesFor<Params...>{}, std::forward<Params>(p)...);
}

template<typename ...Args, std::size_t ...Is>
inline Value FCall::call(indices<Is...>, Args&&... args)
{
    zval zparams[sizeof...(args) ? sizeof...(args) : sizeof...(args) + 1] = { *FCall::paramHelper(std::move(args), zparams[Is])... };
    return this->operator()(sizeof...(args), zparams);
}

}

#endif /* PHPCXX_FCALL_TCC */

#ifndef PHPCXX_FCALL_TCC
#define PHPCXX_FCALL_TCC

#ifndef PHPCXX_FCALL_H
#error "Please do not include this file directly, use fcall.h instead"
#endif

#include "array.h"
#include "value.h"
#include "bailoutrestorer.h"

namespace phpcxx {

template<typename... Params>
static Value call(const char* name, Params&&... p)
{
    {
        BailoutRestorer br;
        JMP_BUF bailout;
        FCall call(name);

        EG(bailout) = &bailout;
        if (EXPECTED(0 == SETJMP(bailout))) {
            return call(std::forward<Params>(p)...);
        }
    }

    _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    ZEND_ASSUME(0); // Unreachable
}

template<typename... Params>
static Value call(const Value& v, Params&&... p)
{
    {
        BailoutRestorer br;
        JMP_BUF bailout;
        FCall call(v.pzval());
        EG(bailout) = &bailout;
        if (EXPECTED(0 == SETJMP(bailout))) {
            return call(std::forward<Params>(p)...);
        }
    }

    _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    ZEND_ASSUME(0); // Unreachable
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

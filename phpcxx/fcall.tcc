#ifndef PHPCXX_FCALL_TCC
#define PHPCXX_FCALL_TCC

#ifndef PHPCXX_FCALL_H
#error "Please do not include this file directly, use fcall.h instead"
#endif

#include "array.h"
#include "value.h"
#include "bailoutrestorer.h"

namespace {

/**
 * @internal
 * @brief Helper function for codecov (`_zend_bailout` is for some reason not marked as `noreturn`)
 * @param filename
 * @param line
 */
[[noreturn]] static void bailout(const char* filename, long int line)
{
    _zend_bailout(const_cast<char*>(filename), line);
    ZEND_ASSUME(0);
}

}

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

    bailout(__FILE__, __LINE__);
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

    bailout(__FILE__, __LINE__);
}

/**
 * @brief Constructs `zval` from `phpcxx::Value`
 * @param[in] v `phpcxx::Value`
 * @return Pointer to `zval`
 */
template<> [[gnu::returns_nonnull]] inline zval* FCall::paramHelper(phpcxx::Value&& v, zval&) { return v.pzval(); }

/**
 * @brief Constructs `zval` from `phpcxx::Array`
 * @param[in] v `phpcxx::Array`
 * @return Pointer to `zval`
 */
template<> [[gnu::returns_nonnull]] inline zval* FCall::paramHelper(phpcxx::Array&& v, zval&) { return v.pzval(); }

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

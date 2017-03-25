#ifndef PHPCXX_CALL_H
#define PHPCXX_CALL_H

#include "helpers.h"

namespace phpcxx {

namespace detail {

template<typename T> zval* paramHelper(T&& v, zval& z) { construct_zval(z, v); Z_TRY_DELREF(z); return &z; }
template<> inline    zval* paramHelper(phpcxx::Value&& v, zval&) { return v.pzval(); }
template<> inline    zval* paramHelper(phpcxx::Array&& v, zval&) { return v.pzval(); }

template<bool throwable, typename ...Args, std::size_t ...Is>
Value call(zval* object, zval* callable, indices<Is...>, Args&&... args)
{
    zval zparams[sizeof...(args) ? sizeof...(args) : sizeof...(args) + 1] = { *paramHelper(std::move(args), zparams[Is])... };
    zval retval;

    zend_fcall_info fci;
    fci.size           = sizeof(fci);
    fci.object         = object ? Z_OBJ_P(object) : nullptr;
    ZVAL_COPY_VALUE(&fci.function_name, callable);
    fci.retval         = &retval;
    fci.param_count    = sizeof...(args);
    fci.params         = zparams;
    fci.no_separation  = 1;
#if PHP_VERSION_ID < 70100
    fci.function_table = CG(function_table);
    fci.symbol_table   = nullptr;
#endif

    if (zend_call_function(&fci, nullptr) == SUCCESS) {
        if (throwable && UNEXPECTED(EG(exception))) {
            i_zval_ptr_dtor(&retval ZEND_FILE_LINE_CC);
            throw PhpException();
        }

        return phpcxx::Value(&retval, CopyPolicy::Wrap);
    }

    return phpcxx::Value();
}

}

template<typename... Params>
phpcxx::Value call(const char* name, Params&&... p)
{
    phpcxx::Value f(name);
    return detail::call<true>(nullptr, f.pzval(), IndicesFor<Params...>{}, std::forward<Params>(p)...);
}

template<typename... Params>
phpcxx::Value call(const Value& f, Params&&... p)
{
    return detail::call<true>(nullptr, f.pzval(), IndicesFor<Params...>{}, std::forward<Params>(p)...);
}

}

#endif /* PHPCXX_CALL_H */

#include <stdexcept>
#include "fcall.h"
#include "callable.h"
#include "value.h"

phpcxx::FCall::FCall(const zend_fcall_info& fci, const zend_fcall_info_cache& fcc)
    : m_fci(fci), m_fcc(fcc)
{
    if (UNEXPECTED(this->m_fci.size != sizeof(zend_fcall_info))) {
        throw std::invalid_argument("Invalid function call information");
    }

    Z_TRY_ADDREF_P(&this->m_fci.function_name);
}

phpcxx::FCall::FCall(Callable c)
{
    if (UNEXPECTED(!c.resolve(this->m_fci, this->m_fcc))) {
        throw std::invalid_argument("Invalid callable");
    }

    Z_TRY_ADDREF_P(&this->m_fci.function_name);
}

phpcxx::FCall::~FCall()
{
    zval_dtor(&this->m_fci.function_name);
#ifdef PHPCXX_DEBUG
    ZVAL_UNDEF(&this->m_fci.function_name);
#endif
}

phpcxx::Value phpcxx::FCall::operator()(vector<Value> args)
{
    vector<zval> v;
    v.reserve(args.size());
    for (std::size_t i=0; i<args.size(); ++i) {
        Value&& x = std::move(args[i]);
        zval z = *x.pzval();
        v.push_back(z);
    }

    return this->operator()(v.size(), v.data());
}

phpcxx::Value phpcxx::FCall::operator()(vector<zval> args)
{
    return this->operator()(args.size(), args.data());
}

phpcxx::Value phpcxx::FCall::operator()(std::size_t n, zval* args)
{
    Value retval;
    zend_fcall_info_argp(&this->m_fci, n, args);
    int status = zend_fcall_info_call(&this->m_fci, &this->m_fcc, retval.pzval(), nullptr);
    zend_fcall_info_args_clear(&this->m_fci, 1);
    if (UNEXPECTED(FAILURE == status)) {
        throw std::runtime_error("Call failed");
    }
    else if (UNEXPECTED(EG(exception))) {
        throw PhpException();
    }

    return retval;
}

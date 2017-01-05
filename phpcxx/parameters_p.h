#ifndef PHPCXX_PARAMETERS_P_H
#define PHPCXX_PARAMETERS_P_H

#include "phpcxx.h"

#include <Zend/zend_API.h>
#include <cassert>
#include "phpexception.h"
#include "vector.h"
#include "value.h"

namespace phpcxx {

class PHPCXX_HIDDEN ParametersPrivate {
    friend class Parameters;
public:
    ParametersPrivate()
    {
        zval* param_ptr       = ZEND_CALL_ARG(EG(current_execute_data), 1);
        std::size_t arg_count = ZEND_CALL_NUM_ARGS(EG(current_execute_data));

        this->m_params.reserve(arg_count);
        zend_function* f = EG(current_execute_data)->func;
        zval* ptr        = param_ptr;

        for (std::size_t i=0; i<arg_count; ++i, ++ptr) {
            zend_check_internal_arg_type(f, i+1, ptr);
            this->m_params.push_back(ptr);
            if (UNEXPECTED(EG(exception))) {
                throw PhpException();
            }
        }
    }

private:
    phpcxx::vector<zval*> m_params;
};

}

#endif /* PHPCXX_PARAMETERS_P_H */

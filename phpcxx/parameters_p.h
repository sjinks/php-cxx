#ifndef PHPCXX_PARAMETERS_P_H
#define PHPCXX_PARAMETERS_P_H

#include "phpcxx.h"

#include <Zend/zend_API.h>
#include <initializer_list>
#include <memory>
#include "phpexception.h"
#include "vector.h"
#include "value.h"

namespace phpcxx {

class PHPCXX_HIDDEN ParametersPrivate {
    friend class Parameters;
public:
    ParametersPrivate(std::initializer_list<Value*> l)
        : m_execute_data(nullptr)
    {
        this->m_params.reserve(l.size());
        for (auto&& v : l) {
            this->m_params.push_back(v->pzval());
        }
    }

    ParametersPrivate(const vector<Value*>& v)
        : m_execute_data(nullptr)
    {
        this->m_params.reserve(v.size());
        for (std::size_t i=0; i<v.size(); ++i) {
            this->m_params.push_back(v[i]->pzval());
        }
    }

    ParametersPrivate(struct _zend_execute_data* execute_data)
        : m_execute_data(execute_data)
    {
        zval* param_ptr       = ZEND_CALL_ARG(execute_data, 1);
        std::size_t arg_count = EX_NUM_ARGS();

        this->m_params.reserve(arg_count);
        for (std::size_t i=0; i<arg_count; ++i, ++param_ptr) {
            this->m_params.push_back(param_ptr);
        }
    }

    bool verify() const
    {
        if (this->m_execute_data) {
            zend_execute_data* execute_data = this->m_execute_data;

            if (EX(func)->common.required_num_args < EX_NUM_ARGS()) {
                zend_wrong_paramers_count_error(EX_NUM_ARGS(), EX(func)->common.required_num_args, EX(func)->common.num_args);
                if (UNEXPECTED(EG(exception))) {
                    throw PhpException();
                }

                return false;
            }

            zend_function* f = EX(func);
            for (std::size_t i=0; i<this->m_params.size(); ++i) {
                zend_check_internal_arg_type(f, i+1, this->m_params[i]);
                if (UNEXPECTED(EG(exception))) {
                    throw PhpException();
                }
            }
        }

        return true;
    }

private:
    zend_execute_data* m_execute_data;
    phpcxx::vector<zval*> m_params;
};

}

#endif /* PHPCXX_PARAMETERS_P_H */

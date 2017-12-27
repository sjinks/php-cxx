#ifndef PHPCXX_PARAMETERS_P_H
#define PHPCXX_PARAMETERS_P_H

#include "phpcxx.h"

#include <initializer_list>
#include <memory>
#include "phpexception.h"
#include "vector.h"
#include "value.h"

namespace phpcxx {

/**
 * @internal
 * @brief Implementation details of @ref phpcxx::Parameters
 */
class PHPCXX_HIDDEN ParametersPrivate {
    friend class Parameters;
public:
    /**
     * @brief Constructor
     * @param l Parameters
     */
    ParametersPrivate(std::initializer_list<Value*> l)
    {
        this->m_params.reserve(l.size());
        for (auto&& v : l) {
            this->m_params.push_back(v->pzval());
        }
    }

    /**
     * @overload
     */
    ParametersPrivate(const vector<Value*>& v)
    {
        this->m_params.reserve(v.size());
        for (std::size_t i=0; i<v.size(); ++i) {
            this->m_params.push_back(v[i]->pzval());
        }
    }

    /**
     * @brief Default constructor
     * @details Constructs the class from `EG(current_execute_data)`
     */
    ParametersPrivate()
    {
        zend_execute_data* execute_data = EG(current_execute_data);
        if (EXPECTED(execute_data)) {
            zval* param_ptr       = ZEND_CALL_ARG(execute_data, 1);
            std::size_t arg_count = EX_NUM_ARGS();

            this->m_params.reserve(arg_count);
            for (std::size_t i=0; i<arg_count; ++i, ++param_ptr) {
                this->m_params.push_back(param_ptr);
            }
        }
    }

    /**
     * @brief Checks whether the number of arguments and their type match the specification of the function
     * @return Check status
     * @throws phpcxx::PhpException if any of `zend_wrong_paramers_count_error()` / `zend_wrong_parameters_count_error()` /
     * `zend_check_internal_arg_type()` throws an exception
     */
    bool verify() const
    {
        if (EG(current_execute_data)) {
            zend_execute_data* execute_data = EG(current_execute_data);

            if (EX_NUM_ARGS() < EX(func)->common.required_num_args) {
#if PHP_VERSION_ID >= 70200
                zend_wrong_parameters_count_error(
                    0,
                    static_cast<int>(EX_NUM_ARGS()),
                    static_cast<int>(EX(func)->common.required_num_args),
                    static_cast<int>(EX(func)->common.num_args)
                );
#elif PHP_VERSION_ID >= 70100
                zend_wrong_parameters_count_error(
                    static_cast<int>(EX_NUM_ARGS()),
                    static_cast<int>(EX(func)->common.required_num_args),
                    static_cast<int>(EX(func)->common.num_args)
                );
#else
                zend_wrong_paramers_count_error(
                    static_cast<int>(EX_NUM_ARGS()),
                    static_cast<int>(EX(func)->common.required_num_args),
                    static_cast<int>(EX(func)->common.num_args)
                );
#endif

                if (UNEXPECTED(EG(exception))) {
                    throw PhpException();
                }

                return false;
            }

            zend_function* f = EX(func);
            for (std::size_t i=0; i<this->m_params.size(); ++i) {
                zend_check_internal_arg_type(f, static_cast<uint32_t>(i+1), this->m_params[i]);
                if (UNEXPECTED(EG(exception))) {
                    throw PhpException();
                }
            }
        }

        return true;
    }

private:
    /**
     * @brief Vector of parameters
     */
    phpcxx::vector<zval*> m_params;
};

}

#endif /* PHPCXX_PARAMETERS_P_H */

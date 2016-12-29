#ifndef PHPCXX_PARAMETERS_P_H
#define PHPCXX_PARAMETERS_P_H

#include "phpcxx.h"

#include <Zend/zend_API.h>
#include <cassert>
#include <initializer_list>
#include "vector.h"
#include "value.h"

namespace phpcxx {

class PHPCXX_HIDDEN ParametersPrivate {
    friend class Parameters;
public:
    ParametersPrivate(int argc)
    {
        assert(argc >= 0);

        this->m_params.reserve(argc);
        zval p[argc];

        zend_function* f = EG(current_execute_data)->func;

        if (EXPECTED(SUCCESS == _zend_get_parameters_array_ex(argc, p))) {
            for (int i=0; i<argc; ++i) {
                zend_check_internal_arg_type(f, i+1, &p[i]);
                if (UNEXPECTED(EG(exception))) {
                    return;
                }
            }

            for (int i=0; i<argc; ++i) {
                this->m_params.emplace_back(&p[i]);
            }

            return;
        }
    }

    ParametersPrivate(std::initializer_list<Value> v)
        : m_params(v)
    {
    }

    ~ParametersPrivate() = default;

private:
    phpcxx::vector<Value> m_params;
};

}

#endif /* PHPCXX_PARAMETERS_P_H */

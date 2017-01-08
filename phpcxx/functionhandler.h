#ifndef PHPCXX_FUNCTIONHANDLER_H
#define PHPCXX_FUNCTIONHANDLER_H

#include "phpcxx.h"

#include <Zend/zend_API.h>

#include "parameters.h"
#include "phpexception.h"
#include "types.h"
#include "value.h"

namespace phpcxx {

class PHPCXX_EXPORT FunctionHandler {
public:
    template<FunctionPrototypeNN f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        f();
        if (EXPECTED(!EG(exception))) {
            ZVAL_NULL(return_value);
        }
        else {
            throw PhpException();
        }
    }

    template<FunctionPrototypeNV f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        Parameters p(execute_data);

        f(p);
        if (EXPECTED(!EG(exception))) {
            ZVAL_NULL(return_value);
        }
        else {
            throw PhpException();
        }
    }

    template<FunctionPrototypeVN f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        Value r = f();
        if (EXPECTED(!EG(exception))) {
            r.assignTo(return_value);
        }
        else {
            throw PhpException();
        }
    }

    template<FunctionPrototypeVV f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        Parameters p(execute_data);
        Value r = f(p);
        if (EXPECTED(!EG(exception))) {
            r.assignTo(return_value);
        }
        else {
            throw PhpException();
        }
    }
};

}

#endif /* FUNCTIONHANDLER_H */

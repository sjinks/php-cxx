#ifndef PHPCXX_FUNCTIONHANDLER_H
#define PHPCXX_FUNCTIONHANDLER_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend_API.h>
}

#include "exception.h"
#include "parameters.h"
#include "types.h"
#include "value.h"

namespace phpcxx {

class PHPCXX_EXPORT FunctionHandler {
public:
    template<FunctionPrototypeNN f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        JMP_BUF* orig_bailout = EG(bailout);
        JMP_BUF bailout;
        bool bailed_out  = false;

        try {
            EG(bailout) = &bailout;
            if (EXPECTED(0 == SETJMP(bailout))) {
                f();
                if (EXPECTED(!EG(exception))) {
                    ZVAL_NULL(return_value);
                }
            }
            else {
                bailed_out = true;
            }
        }
        catch (const std::exception& e) {
            zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
        }
        catch (...) {
            zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception");
        }

        EG(bailout) = orig_bailout;
        if (UNEXPECTED(bailed_out)) {
            _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
        }
    }

    template<FunctionPrototypeNV f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        JMP_BUF* orig_bailout = EG(bailout);
        JMP_BUF bailout;
        bool bailed_out  = false;

        EG(bailout) = &bailout;
        try {
            if (EXPECTED(0 == SETJMP(bailout))) {
                Parameters p(execute_data);

                f(p);
                if (EXPECTED(!EG(exception))) {
                    ZVAL_NULL(return_value);
                }
            }
            else {
                bailed_out = true;
            }
        }
        catch (const std::exception& e) {
            zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
        }
        catch (...) {
            zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception");
        }

        EG(bailout) = orig_bailout;
        if (UNEXPECTED(bailed_out)) {
            _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
        }
    }

    template<FunctionPrototypeVN f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        JMP_BUF* orig_bailout = EG(bailout);
        JMP_BUF bailout;
        bool bailed_out  = false;

        EG(bailout) = &bailout;
        try {
            if (EXPECTED(0 == SETJMP(bailout))) {
                Value r = f();
                if (EXPECTED(!EG(exception))) {
                    r.assignTo(return_value);
                }
            }
            else {
                bailed_out = true;
            }
        }
        catch (const std::exception& e) {
            zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
        }
        catch (...) {
            zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception");
        }

        EG(bailout) = orig_bailout;
        if (UNEXPECTED(bailed_out)) {
            _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
        }
    }

    template<FunctionPrototypeVV f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        JMP_BUF* orig_bailout = EG(bailout);
        JMP_BUF bailout;
        bool bailed_out  = false;

        EG(bailout) = &bailout;
        try {
            if (EXPECTED(0 == SETJMP(bailout))) {
                Parameters p(execute_data);
                Value r = f(p);
                if (EXPECTED(!EG(exception))) {
                    r.assignTo(return_value);
                }
            }
            else {
                bailed_out = true;
            }
        }
        catch (const std::exception& e) {
            zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
        }
        catch (...) {
            zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception");
        }

        EG(bailout) = orig_bailout;
        if (UNEXPECTED(bailed_out)) {
            _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
        }
    }
};

}

#endif /* FUNCTIONHANDLER_H */

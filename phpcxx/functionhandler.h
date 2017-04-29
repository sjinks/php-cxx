#ifndef PHPCXX_FUNCTIONHANDLER_H
#define PHPCXX_FUNCTIONHANDLER_H

#include "phpcxx.h"

#include "bailoutrestorer.h"
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
        {
            BailoutRestorer br;

            try {
                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    f();
                    if (EXPECTED(!EG(exception))) {
                        ZVAL_NULL(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

    template<FunctionPrototypeNV f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                JMP_BUF bailout;
                EG(bailout) = &bailout;
                Parameters p;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    f(p);
                    if (EXPECTED(!EG(exception))) {
                        ZVAL_NULL(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

    template<FunctionPrototypeVN f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    Value r = f();
                    if (EXPECTED(!EG(exception))) {
                        r.assignTo(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

    template<FunctionPrototypeVV f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                Parameters p;
                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    Value r = f(p);
                    if (EXPECTED(!EG(exception))) {
                        r.assignTo(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }


    /**********************************************************************************************/


    template<typename T, MethodPrototypeNN<T> f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                zval* this_ptr = getThis();
                zval* z        = FunctionHandler::checkClass(this_ptr);
                Value vthis(this_ptr);

                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    ((static_cast<T*>(Z_PTR_P(z)))->*f)(vthis);
                    if (EXPECTED(!EG(exception))) {
                        ZVAL_NULL(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

    template<typename T, ConstMethodPrototypeNN<T> f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                zval* this_ptr = getThis();
                zval* z        = FunctionHandler::checkClass(this_ptr);
                Value vthis(this_ptr);

                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    ((static_cast<T*>(Z_PTR_P(z)))->*f)(vthis);
                    if (EXPECTED(!EG(exception))) {
                        ZVAL_NULL(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

    template<typename T, MethodPrototypeVN<T> f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                zval* this_ptr = getThis();
                zval* z        = FunctionHandler::checkClass(this_ptr);
                Value vthis(this_ptr);

                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    Value r = ((static_cast<T*>(Z_PTR_P(z)))->*f)(vthis);
                    if (EXPECTED(!EG(exception))) {
                        r.assignTo(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

    template<typename T, ConstMethodPrototypeVN<T> f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                zval* this_ptr = getThis();
                zval* z        = FunctionHandler::checkClass(this_ptr);
                Value vthis(this_ptr);

                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    Value r = ((static_cast<T*>(Z_PTR_P(z)))->*f)(vthis);
                    if (EXPECTED(!EG(exception))) {
                        r.assignTo(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

    template<typename T, MethodPrototypeNV<T> f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                zval* this_ptr = getThis();
                zval* z        = FunctionHandler::checkClass(this_ptr);
                Value vthis(this_ptr);
                Parameters p(execute_data);

                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    ((static_cast<T*>(Z_PTR_P(z)))->*f)(vthis, p);
                    if (EXPECTED(!EG(exception))) {
                        ZVAL_NULL(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

    template<typename T, ConstMethodPrototypeNV<T> f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                zval* this_ptr = getThis();
                zval* z        = FunctionHandler::checkClass(this_ptr);
                Value vthis(this_ptr);
                Parameters p(execute_data);

                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    ((static_cast<T*>(Z_PTR_P(z)))->*f)(vthis, p);
                    if (EXPECTED(!EG(exception))) {
                        ZVAL_NULL(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

    template<typename T, MethodPrototypeVV<T> f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                zval* this_ptr = getThis();
                zval* z        = FunctionHandler::checkClass(this_ptr);
                Value vthis(this_ptr);
                Parameters p(execute_data);

                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    Value r = ((static_cast<T*>(Z_PTR_P(z)))->*f)(vthis, p);
                    if (EXPECTED(!EG(exception))) {
                        r.assignTo(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

    template<typename T, ConstMethodPrototypeVV<T> f>
    static void handler(struct _zend_execute_data* execute_data, struct _zval_struct* return_value)
    {
        {
            BailoutRestorer br;

            try {
                zval* this_ptr = getThis();
                zval* z        = FunctionHandler::checkClass(this_ptr);
                Value vthis(this_ptr);
                Parameters p(execute_data);

                JMP_BUF bailout;
                EG(bailout) = &bailout;
                if (EXPECTED(0 == SETJMP(bailout))) {
                    Value r = ((static_cast<T*>(Z_PTR_P(z)))->*f)(vthis, p);
                    if (EXPECTED(!EG(exception))) {
                        r.assignTo(return_value);
                    }

                    return;
                }
            }
            catch (phpcxx::PhpException& e) {
                e.passException();
                return;
            }
            catch (const std::exception& e) {
                zend_throw_exception_ex(phpcxx_exception_ce, 0, "Unhandled C++ exception: %s", e.what());
                return;
            }
        }

        _zend_bailout(const_cast<char*>(__FILE__), __LINE__);
    }

private:
    static zval* checkClass(zval* this_ptr)
    {
        if (UNEXPECTED(!this_ptr)) {
            throw std::logic_error("this pointer is NULL");
        }

        zval* z = zend_read_property(Z_OBJCE_P(this_ptr), this_ptr, ZEND_STRL("\xFF"), 1, nullptr);
        if (UNEXPECTED(!z || Z_TYPE_P(z) != IS_PTR)) {
            throw std::logic_error("The class is corrupt");
        }

        return z;
    }

};

}

#endif /* FUNCTIONHANDLER_H */

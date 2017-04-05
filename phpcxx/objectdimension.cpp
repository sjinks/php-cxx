extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_compile.h>
#include <Zend/zend_exceptions.h>
}

#include "objectdimension.h"
#include "phpexception.h"
#include "value.h"

phpcxx::ObjectDimension::operator phpcxx::Value()
{
    zval* container = Z_ISREF_P(this->m_object) ? Z_REFVAL_P(this->m_object) : this->m_object;
    zval dim;
    zval result;

    if (this->m_offset.type == OffsetType::Numeric) {
        ZVAL_LONG(&dim, this->m_offset.k.h);
    }
    else {
        ZVAL_STR(&dim, this->m_offset.k.key);
    }

    // dim is not destroyed intentionally, as it does not actually owns the string

    if (Z_OBJ_HT_P(container)->read_dimension) {
        zval* retval = Z_OBJ_HT_P(container)->read_dimension(container, &dim, BP_VAR_R, &result);
        if (!EG(exception)) {
            if (retval) {
                if (&result != retval) {
                    // If offsetExists returns false, retval will be &EG(uninitialized_zval), result will be garbage
                    ZVAL_COPY(&result, retval);
                }

                if (Z_ISREF(result)) {
                    Z_DELREF(result);
                }

                return Value(&result, CopyPolicy::Copy);
            }

            // This is the (unlikely) case when offsetExists returns IS_UNDEF
            return Value(nullptr);
        }
    }
    else {
        zend_throw_error(NULL, "Cannot use object as array");
    }

    assert(EG(exception));
    throw PhpException();
}

phpcxx::ObjectDimension& phpcxx::ObjectDimension::operator=(const phpcxx::Value& v)
{
    zval* container = Z_ISREF_P(this->m_object) ? Z_REFVAL_P(this->m_object) : this->m_object;
    zval dim;

    if (this->m_offset.type == OffsetType::Numeric) {
        ZVAL_LONG(&dim, this->m_offset.k.h);
    }
    else {
        ZVAL_STR(&dim, this->m_offset.k.key);
    }

    if (Z_OBJ_HT_P(container)->write_dimension) {
        Z_OBJ_HT_P(container)->write_dimension(container, &dim, v.pzval());
    }
    else {
        zend_throw_error(NULL, "Cannot use object as array");
    }

    // dim is not destroyed intentionally, as it does not actually owns the string

    if (EG(exception)) {
        throw PhpException();
    }

    return *this;
}

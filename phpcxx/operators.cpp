extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_operators.h>
}

#include <cstring>
#include "operators.h"
#include "value.h"

bool phpcxx::operator==(const phpcxx::Value& lhs, const phpcxx::Value& rhs)
{
    zval& a = lhs.m_z;
    zval& b = rhs.m_z;
    zval c;

    if (Z_TYPE(a) == IS_LONG) {
        if (Z_TYPE(b) == IS_LONG) {
            return Z_LVAL(a) == Z_LVAL(b);
        }

        if (Z_TYPE(b) == IS_DOUBLE) {
            return static_cast<double>(Z_LVAL(a)) == Z_DVAL(b);
        }
    }
    else if (Z_TYPE(a) == IS_DOUBLE) {
        if (Z_TYPE(b) == IS_DOUBLE) {
            return Z_DVAL(a) == Z_DVAL(b);
        }

        if (Z_TYPE(b) == IS_LONG) {
            return Z_DVAL(a) == static_cast<double>(Z_LVAL(b));
        }
    }
    else if (Z_TYPE(a) == IS_STRING) {
        if (Z_STR(a) == Z_STR(b)) {
            return true;
        }

        if (Z_STRVAL(a)[0] > '9' || Z_STRVAL(b)[0] > '9') {
            if (Z_STRLEN(a) != Z_STRLEN(b)) {
                return false;
            }

            return 0 == std::memcmp(Z_STRVAL(a), Z_STRVAL(b), Z_STRLEN(a));
        }

        return zendi_smart_strcmp(Z_STR(a), Z_STR(b)) == 0;
    }

    compare_function(&c, &a, &b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return Z_LVAL(c) == 0;
}

bool phpcxx::operator<(const phpcxx::Value& lhs, const phpcxx::Value& rhs)
{
    zval& a = lhs.m_z;
    zval& b = rhs.m_z;
    zval c;

    if (Z_TYPE(a) == IS_LONG) {
        if (Z_TYPE(b) == IS_LONG) {
            return Z_LVAL(a) < Z_LVAL(b);
        }

        if (Z_TYPE(b) == IS_DOUBLE) {
            return static_cast<double>(Z_LVAL(a)) < Z_DVAL(b);
        }
    }
    else if (Z_TYPE(a) == IS_DOUBLE) {
        if (Z_TYPE(b) == IS_DOUBLE) {
            return Z_DVAL(a) < Z_DVAL(b);
        }

        if (Z_TYPE(b) == IS_LONG) {
            return Z_DVAL(a) < static_cast<double>(Z_LVAL(b));
        }
    }

    compare_function(&c, &a, &b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return Z_LVAL(c) < 0;
}

bool phpcxx::operator<=(const phpcxx::Value& lhs, const phpcxx::Value& rhs)
{
    zval& a = lhs.m_z;
    zval& b = rhs.m_z;
    zval c;

    if (Z_TYPE(a) == IS_LONG) {
        if (Z_TYPE(b) == IS_LONG) {
            return Z_LVAL(a) <= Z_LVAL(b);
        }

        if (Z_TYPE(b) == IS_DOUBLE) {
            return static_cast<double>(Z_LVAL(a)) <= Z_DVAL(b);
        }
    }
    else if (Z_TYPE(a) == IS_DOUBLE) {
        if (Z_TYPE(b) == IS_DOUBLE) {
            return Z_DVAL(a) <= Z_DVAL(b);
        }

        if (Z_TYPE(b) == IS_LONG) {
            return Z_DVAL(a) <= static_cast<double>(Z_LVAL(b));
        }
    }

    compare_function(&c, &a, &b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return Z_LVAL(c) <= 0;
}

phpcxx::Value phpcxx::operator~(const phpcxx::Value& op)
{
    zval& a = op.m_z;
    zval b;
    bitwise_not_function(&b, &a);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return phpcxx::Value(&b, phpcxx::CopyPolicy::Copy);
}

phpcxx::Value phpcxx::operator!(const phpcxx::Value& op)
{
    zval& a = op.m_z;

    if (Z_TYPE(a) == IS_TRUE) {
        return phpcxx::Value(false);
    }

    if (Z_TYPE_INFO(a) <= IS_TRUE) {
        return phpcxx::Value(true);
    }

    bool b = !i_zend_is_true(&a);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return phpcxx::Value(b);
}


phpcxx::Value phpcxx::pow(phpcxx::Value lhs, const phpcxx::Value& rhs)
{
    zval& a = lhs.m_z;
    zval& b = rhs.m_z;
    zval c;

    pow_function(&c, &a, &b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return phpcxx::Value(&c, phpcxx::CopyPolicy::Copy);
}

phpcxx::Value phpcxx::concat(phpcxx::Value lhs, const phpcxx::Value& rhs)
{
    zval& a = lhs.m_z;
    zval& b = rhs.m_z;
    zval c;

    if (Z_TYPE(a) == IS_STRING && Z_TYPE(b) == IS_STRING) {
        zend_string* op1_str = Z_STR(a);
        zend_string* op2_str = Z_STR(b);

        if (UNEXPECTED(ZSTR_LEN(op1_str) == 0)) {
            ZVAL_STR_COPY(&c, op2_str);
        }
        else if (UNEXPECTED(ZSTR_LEN(op2_str) == 0)) {
            ZVAL_STR_COPY(&c, op1_str);
        }
        else if (!ZSTR_IS_INTERNED(op1_str) && GC_REFCOUNT(op1_str) == 1) {
            std::size_t len  = ZSTR_LEN(op1_str);
            zend_string* str = zend_string_realloc(op1_str, len + ZSTR_LEN(op2_str), 0);
            std::memcpy(ZSTR_VAL(str) + len, ZSTR_VAL(op2_str), ZSTR_LEN(op2_str)+1);
            ZVAL_NEW_STR(&c, str);
        }
        else {
            zend_string* str = zend_string_alloc(ZSTR_LEN(op1_str) + ZSTR_LEN(op2_str), 0);
            std::memcpy(ZSTR_VAL(str), ZSTR_VAL(op1_str), ZSTR_LEN(op1_str));
            std::memcpy(ZSTR_VAL(str) + ZSTR_LEN(op1_str), ZSTR_VAL(op2_str), ZSTR_LEN(op2_str)+1);
            ZVAL_NEW_STR(&c, str);
        }

        return phpcxx::Value(&c, phpcxx::CopyPolicy::Copy);
    }

    concat_function(&c, &a, &b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return phpcxx::Value(&c, phpcxx::CopyPolicy::Copy);
}

int phpcxx::compare(const phpcxx::Value& lhs, const phpcxx::Value& rhs)
{
    zval& a = lhs.m_z;
    zval& b = rhs.m_z;
    zval c;

    compare_function(&c, &a, &b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return static_cast<int>(Z_LVAL(c));
}

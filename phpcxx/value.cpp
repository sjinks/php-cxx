#include <array>
#include <cstdio>
#include <new>
#include <string>
#include <unordered_map>
#include "phpexception.h"
#include "string.h"
#include "value.h"

extern "C" {
#include <Zend/zend_exceptions.h>
}

#define TYPE_PAIR(t1, t2) (((t1) << 4) | (t2))
#undef snprintf

static_assert(sizeof(phpcxx::Value) == sizeof(zval), "sizeof(Value) must be equal to sizeof(zval)");

phpcxx::Value phpcxx::ErrorValue(nullptr);

phpcxx::Value& phpcxx::Value::operator+=(const phpcxx::Value& rhs)
{
    zval* a = &this->m_z;
    zval* b = &rhs.m_z;

    if (this->isRefcounted()) {
        ZVAL_DEREF(a);
        SEPARATE_ZVAL_NOREF(a);
    }

    switch (TYPE_PAIR(Z_TYPE_P(a), Z_TYPE_P(b))) {
        case TYPE_PAIR(IS_LONG, IS_LONG):
            fast_long_add_function(a, a, b);
            return *this;

        case TYPE_PAIR(IS_LONG, IS_DOUBLE):
            ZVAL_DOUBLE(a, static_cast<double>(Z_LVAL_P(a)) + Z_DVAL_P(b));
            return *this;

        case TYPE_PAIR(IS_DOUBLE, IS_LONG):
            ZVAL_DOUBLE(a, Z_DVAL_P(a) + static_cast<double>(Z_LVAL_P(b)));
            return *this;

        case TYPE_PAIR(IS_DOUBLE, IS_DOUBLE):
            ZVAL_DOUBLE(a, Z_DVAL_P(a) + Z_DVAL_P(b));
            return *this;

        case TYPE_PAIR(IS_ARRAY, IS_ARRAY):
            if (a == b) {
                // $a += $a
                return *this;
            }

            zend_hash_merge(Z_ARRVAL_P(a), Z_ARRVAL_P(b), zval_add_ref, 0);
            return *this;
    }

    add_function(a, a, b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator-=(const phpcxx::Value& rhs)
{
    zval* a = &this->m_z;
    zval* b = &rhs.m_z;

    if (this->isRefcounted()) {
        ZVAL_DEREF(a);
        SEPARATE_ZVAL_NOREF(a);
    }

    switch (TYPE_PAIR(Z_TYPE_P(a), Z_TYPE_P(b))) {
        case TYPE_PAIR(IS_LONG, IS_LONG):
            fast_long_sub_function(a, a, b);
            return *this;

        case TYPE_PAIR(IS_LONG, IS_DOUBLE):
            ZVAL_DOUBLE(a, static_cast<double>(Z_LVAL_P(a)) - Z_DVAL_P(b));
            return *this;

        case TYPE_PAIR(IS_DOUBLE, IS_LONG):
            ZVAL_DOUBLE(a, Z_DVAL_P(a) - static_cast<double>(Z_LVAL_P(b)));
            return *this;

        case TYPE_PAIR(IS_DOUBLE, IS_DOUBLE):
            ZVAL_DOUBLE(a, Z_DVAL_P(a) - Z_DVAL_P(b));
            return *this;
    }

    sub_function(a, a, b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator*=(const phpcxx::Value& rhs)
{
    zval* a = &this->m_z;
    zval* b = &rhs.m_z;

    if (this->isRefcounted()) {
        ZVAL_DEREF(a);
        SEPARATE_ZVAL_NOREF(a);
    }

    switch (TYPE_PAIR(Z_TYPE_P(a), Z_TYPE_P(b))) {
        case TYPE_PAIR(IS_LONG, IS_LONG): {
            zend_long overflow;
            ZEND_SIGNED_MULTIPLY_LONG(Z_LVAL_P(a), Z_LVAL_P(b), Z_LVAL_P(a), Z_LVAL_P(a), overflow);
            Z_TYPE_INFO_P(a) = overflow ? IS_DOUBLE : IS_LONG;
            return *this;
        }

        case TYPE_PAIR(IS_LONG, IS_DOUBLE):
            ZVAL_DOUBLE(a, static_cast<double>(Z_LVAL_P(a)) * Z_DVAL_P(b));
            return *this;

        case TYPE_PAIR(IS_DOUBLE, IS_LONG):
            ZVAL_DOUBLE(a, Z_DVAL_P(a) * static_cast<double>(Z_LVAL_P(b)));
            return *this;

        case TYPE_PAIR(IS_DOUBLE, IS_DOUBLE):
            ZVAL_DOUBLE(a, Z_DVAL_P(a) * Z_DVAL_P(b));
            return *this;
    }

    mul_function(a, a, b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator/=(const phpcxx::Value& rhs)
{
    zval* a = &this->m_z;
    zval* b = &rhs.m_z;

    if (this->isRefcounted()) {
        ZVAL_DEREF(a);
        SEPARATE_ZVAL_NOREF(a);
    }

    fast_div_function(a, a, b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator%=(const phpcxx::Value& rhs)
{
    zval* a = &this->m_z;
    zval* b = &rhs.m_z;

    if (this->isRefcounted()) {
        ZVAL_DEREF(a);
        SEPARATE_ZVAL_NOREF(a);
    }

    if (Z_TYPE_P(a) == IS_LONG && Z_TYPE_P(b) == IS_LONG) {
        if (Z_LVAL_P(b) == 0) {
            zend_throw_exception_ex(zend_ce_division_by_zero_error, 0, "Modulo by zero");
            throw phpcxx::PhpException();
        }

        if (Z_LVAL_P(b) == -1) {
            ZVAL_LONG(a, 0);
            return *this;
        }

        ZVAL_LONG(a, Z_LVAL_P(a) % Z_LVAL_P(b));
    }

    mod_function(a, a, b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator|=(const phpcxx::Value& rhs)
{
    zval* a = &this->m_z;
    zval* b = &rhs.m_z;

    if (this->isRefcounted()) {
        ZVAL_DEREF(a);
        SEPARATE_ZVAL_NOREF(a);
    }

    bitwise_or_function(a, a, b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator&=(const phpcxx::Value& rhs)
{
    zval* a = &this->m_z;
    zval* b = &rhs.m_z;

    if (this->isRefcounted()) {
        ZVAL_DEREF(a);
        SEPARATE_ZVAL_NOREF(a);
    }

    bitwise_and_function(a, a, b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator^=(const phpcxx::Value& rhs)
{
    zval* a = &this->m_z;
    zval* b = &rhs.m_z;

    if (this->isRefcounted()) {
        ZVAL_DEREF(a);
        SEPARATE_ZVAL_NOREF(a);
    }

    bitwise_xor_function(a, a, b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator<<=(const phpcxx::Value& rhs)
{
    zval* a = &this->m_z;
    zval* b = &rhs.m_z;

    if (this->isRefcounted()) {
        ZVAL_DEREF(a);
        SEPARATE_ZVAL_NOREF(a);
    }

    shift_left_function(a, a, b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator>>=(const phpcxx::Value& rhs)
{
    zval* a = &this->m_z;
    zval* b = &rhs.m_z;

    if (this->isRefcounted()) {
        ZVAL_DEREF(a);
        SEPARATE_ZVAL_NOREF(a);
    }

    shift_right_function(a, a, b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return *this;
}

bool phpcxx::Value::isIdenticalTo(const phpcxx::Value& rhs)
{
    zval& a = this->m_z;
    zval& b = rhs.m_z;
    bool  c = fast_is_identical_function(&a, &b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return c;
}

bool phpcxx::Value::isNotIdenticalTo(const phpcxx::Value& rhs)
{
    zval& a = this->m_z;
    zval& b = rhs.m_z;
    bool  c = fast_is_not_identical_function(&a, &b);
    if (UNEXPECTED(EG(exception))) {
        throw phpcxx::PhpException();
    }

    return c;
}

static inline zval* separateOrCreateArray(zval* z)
{
    if (Z_ISREF_P(z)) {
        z = Z_REFVAL_P(z);
    }

    if (Z_TYPE_P(z) == IS_ARRAY) {
        SEPARATE_ARRAY(z);
    }
    else if (Z_TYPE_P(z) == IS_OBJECT) {
        // special handling
    }
    else if (Z_TYPE_P(z) == IS_STRING && Z_STRLEN_P(z) != 0) {
        // special handling
    }
    else if (EXPECTED(Z_TYPE_P(z) <= IS_FALSE) || (Z_TYPE_P(z) == IS_STRING && Z_STRLEN_P(z) == 0)) {
        zval_ptr_dtor_nogc(z);
        ZVAL_NEW_ARR(z);
        zend_hash_init(Z_ARRVAL_P(z), 8, NULL, ZVAL_PTR_DTOR, 0);
    }
    else {
        zend_error(E_WARNING, "Cannot use a scalar value as an array");
    }

    return z;
}

phpcxx::Value& phpcxx::Value::operator[](const phpcxx::ZendString& key)
{
    zend_ulong hval;
    if (ZEND_HANDLE_NUMERIC(key.get(), hval)) {
        return this->operator[](static_cast<zend_long>(hval));
    }

    zval* z = separateOrCreateArray(&this->m_z);
    switch (Z_TYPE_P(z)) {
        case IS_ARRAY: {
            zval* retval = zend_hash_find(Z_ARRVAL_P(z), key.get());
            if (retval) {
                if (UNEXPECTED(Z_TYPE_P(retval) == IS_INDIRECT)) {
                    retval = Z_INDIRECT_P(retval);
                    if (UNEXPECTED(Z_TYPE_P(retval) == IS_UNDEF)) {
                        ZVAL_NULL(retval);
                    }
                }
            }
            else {
                retval = zend_hash_add_new(Z_ARRVAL_P(z), key.get(), &EG(uninitialized_zval));
            }

            return *(new(retval) Value(placement_construct));
        }

        case IS_OBJECT:
            zend_error(E_ERROR, "Assignment to object dimension is not implemented yet");
            break;

        case IS_STRING: {
            zend_error(E_WARNING, "Illegal string offset '%s'", ZSTR_VAL(key.get()));
            // offset = 0
            zend_error(E_ERROR, "Assignment to string offset is not implemented yet");
            break;
        }
    }

    return ErrorValue;
}

phpcxx::Value& phpcxx::Value::operator[](zend_long key)
{
    zval* z = separateOrCreateArray(&this->m_z);
    switch (Z_TYPE_P(z)) {
        case IS_ARRAY: {
            zend_ulong h = static_cast<zend_ulong>(key);
            zval* retval = zend_hash_index_find(Z_ARRVAL_P(z), h);
            if (!retval) {
                retval = zend_hash_index_add_new(Z_ARRVAL_P(z), h, &EG(uninitialized_zval));
            }

            return *(new(retval) Value(placement_construct));
        }

        case IS_OBJECT:
            zend_error(E_ERROR, "Assignment to object dimension is not implemented yet");
            break;

        case IS_STRING: {
            if (key < 0) {
                zend_error(E_WARNING, "Illegal string offset:  " ZEND_LONG_FMT, key);
            }
            zend_error(E_ERROR, "Assignment to string offset is not implemented yet");
            break;
        }
    }

    return ErrorValue;
}

phpcxx::Value& phpcxx::Value::operator++()
{
    zval* z = &this->m_z;

    if (Z_TYPE_P(z) == IS_LONG) {
        fast_long_increment_function(z);
    }
    else {
        ZVAL_DEREF(z);
        SEPARATE_ZVAL_NOREF(z);
        increment_function(z);
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator--()
{
    zval* z = &this->m_z;

    if (Z_TYPE_P(z) == IS_LONG) {
        fast_long_decrement_function(z);
    }
    else {
        ZVAL_DEREF(z);
        SEPARATE_ZVAL_NOREF(z);
        decrement_function(z);
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator[](std::nullptr_t)
{
    zval* z = separateOrCreateArray(&this->m_z);
    switch (Z_TYPE_P(z)) {
        case IS_ARRAY: {
            zval* var_ptr = zend_hash_next_index_insert(Z_ARRVAL_P(z), &EG(uninitialized_zval));
            if (UNEXPECTED(!var_ptr)) {
                zend_error(E_WARNING, "Cannot add element to the array as the next element is already occupied");
                return ErrorValue;
            }

            return *(new(var_ptr) Value(placement_construct));
        }

        case IS_OBJECT:
            zend_error(E_ERROR, "Assignment to object dimension is not implemented yet");
            break;

        case IS_STRING:
            zend_throw_error(NULL, "[] operator not supported for strings");
            break;
    }

    return ErrorValue;
}

std::string phpcxx::Value::debugZval() const
{
    std::string s;

    s = "[type=" + std::to_string(this->typeToString(this->type()))
      + " refcounted=" + std::to_string(this->isRefcounted())
      + " isref=" + std::to_string(this->isReference())
      + " refcount=" + std::to_string(this->refCount())
      + "] "
    ;

    s += std::to_string(this->toString());
    return s;
}

phpcxx::string phpcxx::Value::typeToString(phpcxx::Type type)
{
    zend_uchar t = static_cast<zend_uchar>(type);
#if 1
    static std::array<const char*, IS_PTR+1> lut{
        {
            "undefined", "null",     "false",  "true",     "long",      "double",
            "string",    "array",    "object", "resource", "reference", "constant", "constantast",
            "boolean",   "callable",
            "indirect",  "ptr"
        }
    };

    static_assert(IS_UNDEF == 0, "IS_UNDEF should be 0");
    if (t < lut.size()) {
        return lut[t];
    }
#else
    static const std::unordered_map<int, const char*> lut{
        { IS_UNDEF,        "undefined"   },
        { IS_NULL,         "null"        },
        { IS_FALSE,        "false"       },
        { IS_TRUE,         "true"        },
        { IS_LONG,         "long"        },
        { IS_DOUBLE,       "double"      },
        { IS_STRING,       "string"      },
        { IS_ARRAY,        "array"       },
        { IS_OBJECT,       "object"      },
        { IS_RESOURCE,     "resource"    },
        { IS_REFERENCE,    "reference"   },
        { IS_CONSTANT,     "constant"    },
        { IS_CONSTANT_AST, "constantast" },
        { _IS_BOOL,        "boolean"     },
        { IS_CALLABLE,     "callable"    },
        { IS_INDIRECT,     "indirect"    },
        { IS_PTR,          "ptr"         }
    };

    auto it = lut.find(t);
    if (it != lut.end()) {
        return it->second;
    }
#endif

    char buf[8];
    std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned int>(t));
    return string(buf);
}

void get_dimension_address(zval* result, zval* container, zval* dim, int type)
{
    if (Z_TYPE_P(container) == IS_REFERENCE) {
        container = Z_REFVAL_P(container);
    }

    zval* retval;
    switch (Z_TYPE_P(container)) {
        case IS_OBJECT:
            if (Z_OBJ_HT_P(container)->read_dimension) {
                retval = Z_OBJ_HT_P(container)->read_dimension(container, dim, type, result);
                if (BP_VAR_R == type || BP_VAR_IS == type) {
                    if (retval) {
                        if (result != retval) {
                            // If offsetExists returns false, retval will be &EG(uninitialized_zval), result will be garbage
                            ZVAL_COPY(result, retval);
                        }
                    }
                    else {
                        // This is the case when offsetExistsreturns IS_UNDEF
                        ZVAL_NULL(result);
                    }
                }
                else {
                    if (retval && Z_TYPE_P(retval) != IS_UNDEF) {
                        if (!Z_ISREF_P(retval)) {
                            if (Z_REFCOUNTED_P(retval) && Z_REFCOUNT_P(retval) > 1) {
                                if (Z_TYPE_P(retval) != IS_OBJECT) {
                                    Z_DELREF_P(retval);
                                    ZVAL_DUP(result, retval);
                                    retval = result;
                                }
                                else {
                                    ZVAL_COPY_VALUE(result, retval);
                                    retval = result;
                                }
                            }

                            if (Z_TYPE_P(retval) != IS_OBJECT) {
                                // Indirect modification of overloaded element of <class> has no effect
                            }
                        }
                        else if (Z_REFCOUNT_P(retval) == 1) {
                            ZVAL_UNREF(retval);
                        }

                        if (result != retval) {
                            ZVAL_INDIRECT(result, retval);
                        }
                    }
                    else if (retval == &EG(uninitialized_zval)) {
                        // Indirect modification of overloaded element of <class> has no effect
                        ZVAL_NULL(result);
                    }
                    else {
                        ZVAL_INDIRECT(result, &EG(error_zval));
                    }
                }
            }
            else {
                zend_throw_error(NULL, "Cannot use object as array");
                ZVAL_NULL(result);
            }
    }
}

#include <new>
#include <Zend/zend.h>
#include <Zend/zend_API.h>
#include <Zend/zend_operators.h>
#include <Zend/zend_string.h>
#include <array>
#include <cstdio>
#include <string>
#include <unordered_map>
#include "string.h"
#include "value.h"

#undef snprintf

static_assert(sizeof(phpcxx::Value) == sizeof(zval), "sizeof(Value) must be equal to sizeof(zval)");

phpcxx::Value phpcxx::ErrorValue(nullptr);

phpcxx::Value& phpcxx::Value::operator++()
{
    zval& z = this->m_z;

    if (Z_TYPE(z) == IS_LONG) {
        fast_long_increment_function(&z);
    }
    else {
        ZVAL_DEREF(&z);
        SEPARATE_ZVAL_NOREF(&z);
        increment_function(&z);
    }

    return *this;
}

phpcxx::Value& phpcxx::Value::operator--()
{
    zval& z = this->m_z;

    if (Z_TYPE(z) == IS_LONG) {
        fast_long_decrement_function(&z);
    }
    else {
        ZVAL_DEREF(&z);
        SEPARATE_ZVAL_NOREF(&z);
        decrement_function(&z);
    }

    return *this;
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

            return *(new(retval) Value(placement_construction_t()));
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

            return *(new(retval) Value(placement_construction_t()));
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

            return *(new(var_ptr) Value(placement_construction_t()));
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

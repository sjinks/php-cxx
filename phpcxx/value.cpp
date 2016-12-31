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

phpcxx::Value phpcxx::Value::operator[](const phpcxx::ZendString& key)
{
    zval* z = this->maybeDeref();
    if (this->type() != Type::Array) {
        convert_to_array(z);
    }

    zval* res = zend_symtable_find(Z_ARRVAL_P(z), key.get());
    if (!res) {
        zval tmp;
        construct_zval(tmp, nullptr);
        res = zend_symtable_update(Z_ARRVAL_P(z), key.get(), &tmp);
        if (UNEXPECTED(!res)) {
            zval_dtor(&tmp);
            return Value();
        }
    }

    return Value(res, CopyPolicy::Reference);
}

phpcxx::Value phpcxx::Value::operator[](zend_long key)
{
    zval* z = this->maybeDeref();
    if (this->type() != Type::Array) {
        convert_to_array(z);
    }

    zval* res = zend_hash_index_find(Z_ARRVAL_P(z), static_cast<zend_ulong>(key));
    if (!res) {
        zval tmp;
        construct_zval(tmp, nullptr);
        res = zend_hash_index_update(Z_ARRVAL_P(z), static_cast<zend_ulong>(key), &tmp);
        if (UNEXPECTED(!res)) {
            zval_dtor(&tmp);
            return Value();
        }
    }

    return Value(res, CopyPolicy::Reference);
}

phpcxx::Value phpcxx::Value::operator[](std::nullptr_t)
{
    zval* z = this->maybeDeref();
    if (this->type() != Type::Array) {
        convert_to_array(z);
    }

    zval tmp;
    construct_zval(tmp, nullptr);
    zval* res = zend_hash_next_index_insert(Z_ARRVAL_P(z), &tmp);
    if (EXPECTED(res)) {
        return Value(res, CopyPolicy::Reference);
    }

    zval_dtor(&tmp);
    return Value();
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

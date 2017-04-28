#include <cassert>
#include <stdexcept>

extern "C" {
#include <Zend/zend_API.h>
}

#include "array.h"
#include "helpers.h"
#include "string.h"
#include "value.h"
#include "zendstring.h"

phpcxx::Array::Array()
{
    array_init(&this->m_z);
}

phpcxx::Array::Array(zval* z)
{
    assert(z != nullptr);

    if (Z_ISREF_P(z) && Z_TYPE_P(Z_REFVAL_P(z)) == IS_ARRAY) {
        ZVAL_COPY(&this->m_z, Z_REFVAL_P(z));
    }
    else {
        ZVAL_COPY(&this->m_z, z);
        if (Z_TYPE_P(z) != IS_ARRAY) {
            convert_to_array(&this->m_z);
        }
    }
}

phpcxx::Array::Array(Value& v)
    : Array(v.pzval())
{
}

phpcxx::Array::Array(Array& other)
{
    ZVAL_COPY(&this->m_z, &other.m_z);
}

phpcxx::Array::Array(Array&& other)
{
    ZVAL_UNDEF(&this->m_z);
    std::swap(this->m_z, other.m_z);
}

phpcxx::Array::~Array()
{
    i_zval_ptr_dtor(&this->m_z ZEND_FILE_LINE_CC);
#ifdef PHPCXX_DEBUG
    ZVAL_UNDEF(&this->m_z);
#endif
}

phpcxx::Array& phpcxx::Array::operator=(Array& other)
{
    phpcxx::assign(&this->m_z, &other.m_z);
    return *this;
}

phpcxx::Array& phpcxx::Array::operator=(zval* z)
{
    assert(z != nullptr);
    ZVAL_DEREF(z);
    if (Z_TYPE_P(z) != IS_ARRAY) {
        throw std::invalid_argument("zval is not array");
    }

    phpcxx::assign(&this->m_z, z);
    return *this;
}

phpcxx::Value& phpcxx::Array::operator[](std::nullptr_t)
{
    assert(Z_TYPE(this->m_z) == IS_ARRAY);
    SEPARATE_ARRAY(&this->m_z);

    zval* var_ptr = zend_hash_next_index_insert(Z_ARRVAL(this->m_z), &EG(uninitialized_zval));
    if (UNEXPECTED(!var_ptr)) {
        throw std::runtime_error("Cannot add element to the array as the next element is already occupied");
    }

    return *(new(var_ptr) Value(placement_construct));
}

phpcxx::Value& phpcxx::Array::operator[](zend_long idx)
{
    assert(Z_TYPE(this->m_z) == IS_ARRAY);
    SEPARATE_ARRAY(&this->m_z);

    zend_ulong h = static_cast<zend_ulong>(idx);
    zval* retval = zend_hash_index_find(Z_ARRVAL(this->m_z), h);
    if (!retval) {
        retval = zend_hash_index_add_new(Z_ARRVAL(this->m_z), h, &EG(uninitialized_zval));
        if (UNEXPECTED(!retval)) {
            throw std::runtime_error("Cannot add element to the array as the next element is already occupied");
        }
    }

    return *(new(retval) Value(placement_construct));
}

phpcxx::Value& phpcxx::Array::operator[](const Value& key)
{
    zval* z = key.pzval();

    while (true) {
        switch (Z_TYPE_P(z)) {
            case IS_STRING:    return this->operator[](Z_STR_P(z));
            case IS_LONG:      return this->operator[](Z_LVAL_P(z));
            case IS_DOUBLE:    return this->operator[](zend_dval_to_lval(Z_DVAL_P(z)));
            case IS_TRUE:      return this->operator[](static_cast<zend_long>(1l));
            case IS_FALSE:     return this->operator[](static_cast<zend_long>(0l));
            case IS_RESOURCE:  return this->operator[](Z_RES_HANDLE_P(z));
            case IS_UNDEF:     return this->operator[](ZSTR_EMPTY_ALLOC());
            case IS_NULL:      return this->operator[](ZSTR_EMPTY_ALLOC());
            case IS_REFERENCE:
                z = Z_REFVAL_P(z);
                break;

            default:
                throw std::invalid_argument("Illegal offset type");
        }
    }
}

phpcxx::Value& phpcxx::Array::operator[](zend_string* key)
{
    zend_ulong hval;
    if (ZEND_HANDLE_NUMERIC(key, hval)) {
        return this->operator[](static_cast<zend_long>(hval));
    }

    assert(Z_TYPE(this->m_z) == IS_ARRAY);
    SEPARATE_ARRAY(&this->m_z);

    zval* retval = zend_hash_find(Z_ARRVAL(this->m_z), key);
    if (retval) {
        if (UNEXPECTED(Z_TYPE_P(retval) == IS_INDIRECT)) {
            retval = Z_INDIRECT_P(retval);
            if (UNEXPECTED(Z_TYPE_P(retval) == IS_UNDEF)) {
                ZVAL_NULL(retval);
            }
        }
    }
    else {
        retval = zend_hash_add_new(Z_ARRVAL(this->m_z), key, &EG(uninitialized_zval));
        if (UNEXPECTED(!retval)) {
            throw std::runtime_error("Cannot add element to the array as the next element is already occupied");
        }
    }

    return *(new(retval) Value(placement_construct));
}

std::size_t phpcxx::Array::size() const
{
    assert(Z_TYPE(this->m_z) == IS_ARRAY);
    return zend_hash_num_elements(Z_ARRVAL(this->m_z));
}

bool phpcxx::Array::isset(zend_long idx) const
{
    assert(Z_TYPE(this->m_z) == IS_ARRAY);
    return zend_hash_index_exists(Z_ARRVAL(this->m_z), static_cast<zend_ulong>(idx));
}

bool phpcxx::Array::isset(const Value& key) const
{
    zval* z = key.pzval();

    while (true) {
        switch (Z_TYPE_P(z)) {
            case IS_STRING:    return this->isset(Z_STR_P(z));
            case IS_LONG:      return this->isset(Z_LVAL_P(z));
            case IS_DOUBLE:    return this->isset(zend_dval_to_lval(Z_DVAL_P(z)));
            case IS_TRUE:      return this->isset(static_cast<zend_long>(1));
            case IS_FALSE:     return this->isset(static_cast<zend_long>(0));
            case IS_RESOURCE:  return this->isset(Z_RES_HANDLE_P(z));
            case IS_UNDEF:     return this->isset(ZSTR_EMPTY_ALLOC());
            case IS_NULL:      return this->isset(ZSTR_EMPTY_ALLOC());
            case IS_REFERENCE:
                z = Z_REFVAL_P(z);
                break;

            default:
                throw std::invalid_argument("Illegal offset type");
        }
    }
}

bool phpcxx::Array::isset(zend_string* key) const
{
    zend_ulong hval;
    if (ZEND_HANDLE_NUMERIC(key, hval)) {
        return this->isset(static_cast<zend_long>(hval));
    }

    assert(Z_TYPE(this->m_z) == IS_ARRAY);
    return zend_hash_exists(Z_ARRVAL(this->m_z), key);
}

void phpcxx::Array::unset(zend_long idx)
{
    assert(Z_TYPE(this->m_z) == IS_ARRAY);
    SEPARATE_ARRAY(&this->m_z);
    zend_hash_index_del(Z_ARRVAL(this->m_z), static_cast<zend_ulong>(idx));
}

void phpcxx::Array::unset(const Value& key)
{
    zval* z = key.pzval();

    while (true) {
        switch (Z_TYPE_P(z)) {
            case IS_STRING:    return this->unset(Z_STR_P(z));
            case IS_LONG:      return this->unset(Z_LVAL_P(z));
            case IS_DOUBLE:    return this->unset(zend_dval_to_lval(Z_DVAL_P(z)));
            case IS_TRUE:      return this->unset(static_cast<zend_long>(1));
            case IS_FALSE:     return this->unset(static_cast<zend_long>(0));
            case IS_RESOURCE:  return this->unset(Z_RES_HANDLE_P(z));
            case IS_UNDEF:     return this->unset(ZSTR_EMPTY_ALLOC());
            case IS_NULL:      return this->unset(ZSTR_EMPTY_ALLOC());
            case IS_REFERENCE:
                z = Z_REFVAL_P(z);
                break;

            default:
                throw std::invalid_argument("Illegal offset type");
        }
    }
}

void phpcxx::Array::unset(zend_string* key)
{
    zend_ulong hval;
    if (ZEND_HANDLE_NUMERIC(key, hval)) {
        return this->unset(static_cast<zend_long>(hval));
    }

    assert(Z_TYPE(this->m_z) == IS_ARRAY);
    SEPARATE_ARRAY(&this->m_z);

    HashTable* ht = Z_ARRVAL(this->m_z);
    if (UNEXPECTED(ht == &EG(symbol_table))) {
        zend_delete_global_variable(key);
    }
    else {
        zend_hash_del(ht, key);
    }
}

phpcxx::Value& phpcxx::Array::operator[](const char* key)
{
    return this->operator[](ZendString(key).get());
}

phpcxx::Value& phpcxx::Array::operator[](const string& key)
{
    return this->operator[](ZendString(key).get());
}

phpcxx::Value& phpcxx::Array::operator[](const ZendString& key)
{
    return this->operator[](key.get());
}

bool phpcxx::Array::isset(const char* key) const
{
    return this->isset(ZendString(key).get());
}

bool phpcxx::Array::isset(const string& key) const
{
    return this->isset(ZendString(key).get());
}

bool phpcxx::Array::isset(const ZendString& key) const
{
    return this->isset(key.get());
}

void phpcxx::Array::unset(const char* key)
{
    this->unset(ZendString(key).get());
}

void phpcxx::Array::unset(const string& key)
{
    this->unset(ZendString(key).get());
}

void phpcxx::Array::unset(const ZendString& key)
{
    this->unset(key.get());
}

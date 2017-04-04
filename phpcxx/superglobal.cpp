#include <cassert>
#include <string>
#include <main/php_variables.h>
#include <Zend/zend_compile.h>
#include "superglobal.h"

phpcxx::SuperGlobal phpcxx::orig_POST(TRACK_VARS_POST);
phpcxx::SuperGlobal phpcxx::orig_GET(TRACK_VARS_GET);
phpcxx::SuperGlobal phpcxx::orig_COOKIE(TRACK_VARS_COOKIE);
phpcxx::SuperGlobal phpcxx::orig_SERVER(TRACK_VARS_SERVER);
phpcxx::SuperGlobal phpcxx::orig_ENV(TRACK_VARS_ENV);
phpcxx::SuperGlobal phpcxx::orig_FILES(TRACK_VARS_FILES);
phpcxx::SuperGlobal phpcxx::_POST("_POST");
phpcxx::SuperGlobal phpcxx::_GET("_GET");
phpcxx::SuperGlobal phpcxx::_COOKIE("_COOKIE");
phpcxx::SuperGlobal phpcxx::_SERVER("_SERVER");
phpcxx::SuperGlobal phpcxx::_ENV("_ENV");
phpcxx::SuperGlobal phpcxx::_FILES("_FILES");
phpcxx::SuperGlobal phpcxx::_REQUEST("_REQUEST");
phpcxx::SuperGlobal phpcxx::_SESSION("_SESSION");
phpcxx::SuperGlobal phpcxx::GLOBALS("GLOBALS");

static const char* superglobal_idx_lookup[NUM_TRACK_VARS] = {
    "_POST",
    "_GET",
    "_COOKIE",
    "_SERVER",
    "_ENV",
    "_FILES"
};

static zval* getArrayZval(zval* z)
{
    if (z) {
        if (IS_ARRAY == Z_TYPE_P(z)) {
            return z;
        }

        if (Z_ISREF_P(z) && Z_TYPE_P(Z_REFVAL_P(z)) == IS_ARRAY) {
            return Z_REFVAL_P(z);
        }
    }

    return nullptr;
}

static zval* getArrayZvalOrThrow(zval *z)
{
    z = getArrayZval(z);
    if (UNEXPECTED(!z)) {
        throw std::runtime_error("Superglobal is not an array");
    }

    return z;
}

phpcxx::SuperGlobal::SuperGlobal(int idx)
    : m_name(), m_z(nullptr), m_idx(idx)
{
    assert(idx >= 0 && idx < NUM_TRACK_VARS);
    this->m_name = superglobal_idx_lookup[idx];
}

phpcxx::SuperGlobal::SuperGlobal(const char* name)
    : m_name(name), m_z(nullptr), m_idx(-1)
{
}

zval* phpcxx::SuperGlobal::pzval() const
{
    if (!this->m_z) {
        zend_bool is_auto_global = zend_is_auto_global_str(const_cast<char*>(this->m_name), std::strlen(this->m_name));
        if (is_auto_global) {
            if (this->m_idx != -1) {
                this->m_z = &PG(http_globals)[this->m_idx];
            }
            else {
                this->m_z = zend_hash_str_find_ind(&EG(symbol_table), this->m_name, std::strlen(this->m_name));
            }
        }
    }

    return this->m_z;
}

phpcxx::Value& phpcxx::SuperGlobal::get() const
{
    zval* z = this->pzval();
    if (!z) {
        throw std::runtime_error(this->m_name + std::string(" is not a known superglobal"));
    }

    return *(new(z) phpcxx::Value(phpcxx::placement_construct));
}

std::size_t phpcxx::SuperGlobal::size() const
{
    zval* z = this->pzval();
    return z && IS_ARRAY == Z_TYPE_P(z) ? zend_hash_num_elements(Z_ARRVAL_P(z)) : 0;
}

phpcxx::Type phpcxx::SuperGlobal::type() const
{
    zval* z = this->pzval();
    if (EXPECTED(z != nullptr)) {
        return static_cast<Type>(Z_TYPE_P(z));
    }

    return phpcxx::Type::Undefined;
}

phpcxx::Value& phpcxx::SuperGlobal::operator[](std::nullptr_t)
{
    zval* z = getArrayZvalOrThrow(this->pzval());

    SEPARATE_ARRAY(z);
    zval* var_ptr = zend_hash_next_index_insert(Z_ARRVAL_P(z), &EG(uninitialized_zval));
    if (UNEXPECTED(!var_ptr)) {
        throw std::runtime_error("Cannot add element to the array as the next element is already occupied");
    }

    return *(new(var_ptr) Value(placement_construct));
}

phpcxx::Value& phpcxx::SuperGlobal::operator[](zend_long idx)
{
    zval* z = getArrayZvalOrThrow(this->pzval());

    zend_ulong h = static_cast<zend_ulong>(idx);
    zval* retval = zend_hash_index_find(Z_ARRVAL_P(z), h);
    if (!retval) {
        SEPARATE_ARRAY(z);
        retval = zend_hash_index_add_new(Z_ARRVAL_P(z), h, &EG(uninitialized_zval));
        if (UNEXPECTED(!retval)) {
            throw std::runtime_error("Cannot add element to the array as the next element is already occupied");
        }
    }

    return *(new(retval) Value(placement_construct));
}

phpcxx::Value& phpcxx::SuperGlobal::operator[](const phpcxx::Value& key)
{
    zval* z = key.pzval();

    while (true) {
        switch (key.type()) {
            case Type::String:    return this->operator[](Z_STR_P(z));
            case Type::Integer:   return this->operator[](Z_LVAL_P(z));
            case Type::Double:    return this->operator[](zend_dval_to_lval(Z_DVAL_P(z)));
            case Type::True:      return this->operator[](static_cast<zend_long>(1));
            case Type::False:     return this->operator[](static_cast<zend_long>(0));
            case Type::Resource:  return this->operator[](Z_RES_HANDLE_P(z));
            case Type::Undefined: return this->operator[](ZSTR_EMPTY_ALLOC());
            case Type::Null:      return this->operator[](ZSTR_EMPTY_ALLOC());
            case Type::Reference:
                z = Z_REFVAL_P(z);
                break;

            default:
                throw std::runtime_error("Illegal offset type");
        }
    }
}

phpcxx::Value& phpcxx::SuperGlobal::operator[](zend_string* key)
{
    zend_ulong hval;
    if (ZEND_HANDLE_NUMERIC(key, hval)) {
        return this->operator[](static_cast<zend_long>(hval));
    }

    zval* z      = getArrayZvalOrThrow(this->pzval());
    zval* retval = zend_hash_find_ind(Z_ARRVAL_P(z), key);
    if (retval) {
        if (UNEXPECTED(Z_TYPE_P(retval) == IS_UNDEF)) {
            ZVAL_NULL(retval);
        }
    }
    else {
        SEPARATE_ARRAY(z);
        retval = zend_hash_add_new(Z_ARRVAL_P(z), key, &EG(uninitialized_zval));
        if (UNEXPECTED(!retval)) {
            throw std::runtime_error("Cannot add element to the array as the next element is already occupied");
        }
    }

    return *(new(retval) Value(placement_construct));
}

bool phpcxx::SuperGlobal::isset(zend_long idx) const
{
    zval* z = getArrayZval(this->pzval());
    if (z) {
        return zend_hash_index_exists(Z_ARRVAL_P(z), static_cast<zend_ulong>(idx));
    }

    return false;
}

bool phpcxx::SuperGlobal::isset(const Value& key) const
{
    zval* z = key.pzval();

    while (true) {
        switch (key.type()) {
            case Type::String:    return this->isset(Z_STR_P(z));
            case Type::Integer:   return this->isset(Z_LVAL_P(z));
            case Type::Double:    return this->isset(zend_dval_to_lval(Z_DVAL_P(z)));
            case Type::True:      return this->isset(static_cast<zend_long>(1));
            case Type::False:     return this->isset(static_cast<zend_long>(0));
            case Type::Resource:  return this->isset(Z_RES_HANDLE_P(z));
            case Type::Undefined: return this->isset(ZSTR_EMPTY_ALLOC());
            case Type::Null:      return this->isset(ZSTR_EMPTY_ALLOC());
            case Type::Reference:
                z = Z_REFVAL_P(z);
                break;

            default:
                throw std::runtime_error("Illegal offset type");
        }
    }
}

bool phpcxx::SuperGlobal::isset(zend_string* key) const
{
    zend_ulong hval;
    if (ZEND_HANDLE_NUMERIC(key, hval)) {
        return this->isset(static_cast<zend_long>(hval));
    }

    zval* z = getArrayZval(this->pzval());
    return z ? zend_hash_exists(Z_ARRVAL_P(z), key) : false;
}

void phpcxx::SuperGlobal::unset(zend_long idx)
{
    zval* z = getArrayZval(this->pzval());
    if (z) {
        SEPARATE_ARRAY(z);
        zend_hash_index_del(Z_ARRVAL_P(z), static_cast<zend_ulong>(idx));
    }
}

void phpcxx::SuperGlobal::unset(const Value& key)
{
    zval* z = key.pzval();

    while (true) {
        switch (key.type()) {
            case Type::String:    return this->unset(Z_STR_P(z));
            case Type::Integer:   return this->unset(Z_LVAL_P(z));
            case Type::Double:    return this->unset(zend_dval_to_lval(Z_DVAL_P(z)));
            case Type::True:      return this->unset(static_cast<zend_long>(1));
            case Type::False:     return this->unset(static_cast<zend_long>(0));
            case Type::Resource:  return this->unset(Z_RES_HANDLE_P(z));
            case Type::Undefined: return this->unset(ZSTR_EMPTY_ALLOC());
            case Type::Null:      return this->unset(ZSTR_EMPTY_ALLOC());
            case Type::Reference:
                z = Z_REFVAL_P(z);
                break;

            default:
                throw std::runtime_error("Illegal offset type");
        }
    }
}

void phpcxx::SuperGlobal::unset(zend_string* key)
{
    zend_ulong hval;
    if (ZEND_HANDLE_NUMERIC(key, hval)) {
        return this->unset(static_cast<zend_long>(hval));
    }

    zval* z = getArrayZval(this->pzval());
    if (z) {
        SEPARATE_ARRAY(z);
        zend_hash_del_ind(Z_ARRVAL_P(z), key);
    }
}

phpcxx::Value& phpcxx::SuperGlobal::operator[](const char* key)
{
    return this->operator[](ZendString(key).get());
}

phpcxx::Value& phpcxx::SuperGlobal::operator[](const string& key)
{
    return this->operator[](ZendString(key).get());
}

phpcxx::Value& phpcxx::SuperGlobal::operator[](const ZendString& key)
{
    return this->operator[](key.get());
}

bool phpcxx::SuperGlobal::isset(const char* key) const
{
    return this->isset(ZendString(key).get());
}

bool phpcxx::SuperGlobal::isset(const string& key) const
{
    return this->isset(ZendString(key).get());
}

bool phpcxx::SuperGlobal::isset(const ZendString& key) const
{
    return this->isset(key.get());
}

void phpcxx::SuperGlobal::unset(const char* key)
{
    this->unset(ZendString(key).get());
}

void phpcxx::SuperGlobal::unset(const string& key)
{
    this->unset(ZendString(key).get());
}

void phpcxx::SuperGlobal::unset(const ZendString& key)
{
    this->unset(key.get());
}

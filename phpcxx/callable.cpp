#include <cstring>
#include <stdexcept>

extern "C" {
#include <Zend/zend_API.h>
}

#include "callable.h"

phpcxx::Callable::Callable(const char* name)
{
    ZVAL_STRING(&this->m_z, name);
}

phpcxx::Callable::Callable(const char* cls, const char* name)
{
    array_init_size(&this->m_z, 2);
    zend_hash_real_init(Z_ARRVAL(this->m_z), 1);
    ZEND_HASH_FILL_PACKED(Z_ARRVAL(this->m_z)) {
        zval tmp;
        ZVAL_STRING(&tmp, cls);
        ZEND_HASH_FILL_ADD(&tmp);
        ZVAL_STRING(&tmp, name);
        ZEND_HASH_FILL_ADD(&tmp);
    } ZEND_HASH_FILL_END();
}

phpcxx::Callable::Callable(zval* obj, const char* name)
{
    ZVAL_DEREF(obj);
    if (EXPECTED(Z_TYPE_P(obj) == IS_STRING || Z_TYPE_P(obj) == IS_OBJECT)) {
        array_init_size(&this->m_z, 2);
        zend_hash_real_init(Z_ARRVAL(this->m_z), 1);
        ZEND_HASH_FILL_PACKED(Z_ARRVAL(this->m_z)) {
            zval tmp;
            Z_ADDREF_P(obj);
            ZEND_HASH_FILL_ADD(obj);
            ZVAL_STRING(&tmp, name);
            ZEND_HASH_FILL_ADD(&tmp);
        } ZEND_HASH_FILL_END();
    }
    else {
        ZVAL_UNDEF(&this->m_z);
        throw std::invalid_argument("obj must be either string or object");
    }
}

phpcxx::Callable::Callable(zval* arg)
{
    ZVAL_DEREF(arg);
    switch (Z_TYPE_P(arg)) {
        case IS_STRING:     // function name
        case IS_OBJECT:     // closure
            ZVAL_COPY(&this->m_z, arg);
            return;

        case IS_ARRAY: {
            zval* obj    = zend_hash_index_find(Z_ARRVAL_P(arg), 0);
            zval* method = zend_hash_index_find(Z_ARRVAL_P(arg), 1);
            if (obj && method) {
                ZVAL_DEREF(obj);
                ZVAL_DEREF(method);

                if ((Z_TYPE_P(obj) == IS_OBJECT || Z_TYPE_P(obj) == IS_STRING) && Z_TYPE_P(method) == IS_STRING) {
                    array_init_size(&this->m_z, 2);
                    zend_hash_real_init(Z_ARRVAL(this->m_z), 1);
                    ZEND_HASH_FILL_PACKED(Z_ARRVAL(this->m_z)) {
                        Z_ADDREF_P(obj);
                        Z_ADDREF_P(method);
                        ZEND_HASH_FILL_ADD(obj);
                        ZEND_HASH_FILL_ADD(method);
                    } ZEND_HASH_FILL_END();
                    return;
                }
            }
            break;
        }
    }

    throw std::invalid_argument("Invalid callable");
}

phpcxx::Callable::~Callable()
{
    zval_dtor(&this->m_z);
#ifdef PHPCXX_DEBUG
    ZVAL_UNDEF(&this->m_z);
#endif
}

phpcxx::Callable::Callable(Callable&& other)
{
    ZVAL_UNDEF(&this->m_z);
    std::swap(this->m_z, other.m_z);
}

bool phpcxx::Callable::resolve(zend_fcall_info& fci, zend_fcall_info_cache& fcc)
{
    if (SUCCESS == zend_fcall_info_init(&this->m_z, IS_CALLABLE_CHECK_SILENT, &fci, &fcc, nullptr, nullptr)) {
        return true;
    }

    std::memset(&fci, 0, sizeof(fci));
    std::memset(&fcc, 0, sizeof(fcc));
    return false;
}

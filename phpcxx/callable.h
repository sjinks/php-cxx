#ifndef PHPCXX_CALLABLE_H
#define PHPCXX_CALLABLE_H

#include "phpcxx.h"

#include <stdexcept>

extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_API.h>
}

namespace phpcxx {

class PHPCXX_EXPORT Callable {
public:
    [[gnu::nonnull]] Callable(const char* name)
    {
        ZVAL_STRING(&this->m_z, name);
    }

    [[gnu::nonnull]] Callable(const char* cls, const char* name)
    {
        array_init_size(&this->m_z, 2);
        add_next_index_string(&this->m_z, cls);
        add_next_index_string(&this->m_z, name);
    }

    [[gnu::nonnull]] Callable(zval* obj, const char* name)
    {
        ZVAL_DEREF(obj);
        if (EXPECTED(Z_TYPE_P(obj) == IS_STRING || Z_TYPE_P(obj) == IS_OBJECT)) {
            array_init_size(&this->m_z, 2);
            Z_ADDREF_P(obj);
            add_next_index_zval(&this->m_z, obj);
            add_next_index_string(&this->m_z, name);
        }
        else {
            ZVAL_UNDEF(&this->m_z);
            throw std::invalid_argument("obj must be either string or object");
        }
    }

    [[gnu::nonnull]] Callable(zval* arg)
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

                        Z_ADDREF_P(obj);    add_next_index_zval(&this->m_z, obj);
                        Z_ADDREF_P(method); add_next_index_zval(&this->m_z, method);
                        return;
                    }
                }
                break;
            }
        }

        throw std::invalid_argument("Invalid callable");
    }

    ~Callable()
    {
        zval_dtor(&this->m_z);
#ifdef PHPCXX_DEBUG
        ZVAL_UNDEF(&this->m_z);
#endif
    }

    Callable(Callable&& other)
    {
        ZVAL_UNDEF(&this->m_z);
        std::swap(this->m_z, other.m_z);
    }

    bool resolve(zend_fcall_info& fci, zend_fcall_info_cache& fcc) const
    {
        if (EXPECTED(Z_TYPE(this->m_z) != IS_UNDEF)) {
            int res = zend_fcall_info_init(&this->m_z, IS_CALLABLE_CHECK_SILENT, &fci, &fcc, nullptr, nullptr);
            return (SUCCESS == res);
        }

        return false;
    }

private:
    mutable zval m_z;
};

}



#endif /* PHPCXX_CALLABLE_H */

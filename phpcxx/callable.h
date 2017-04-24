#ifndef PHPCXX_CALLABLE_H
#define PHPCXX_CALLABLE_H

#include "phpcxx.h"

#include <stdexcept>

extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_API.h>
}

namespace phpcxx {

/**
 * @brief Callable
 * @see http://php.net/manual/en/language.types.callable.php
 */
class PHPCXX_EXPORT Callable {
public:
    /**
     * @brief Constructs a new callable from a string
     * @param name Callable (function name or static class method)
     */
    [[gnu::nonnull]] Callable(const char* name)
    {
        ZVAL_STRING(&this->m_z, name);
    }

    /**
     * @brief Constructs a new callable (static class method)
     * @param cls Class name
     * @param name Method name
     */
    [[gnu::nonnull]] Callable(const char* cls, const char* name)
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

    /**
     * @brief Constructs a new callable (possibly class method)
     * @param obj Class instance
     * @param name Method name
     * @throws std::invalid_argument if @a obj is neither (a reference to) an object nor (a reference to) a string
     */
    [[gnu::nonnull]] Callable(zval* obj, const char* name)
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

    /**
     * @brief Constructs a new callable from `zval*`
     * @param arg Callable
     * @throws std::invalid_argument if @a arg is not a valid callable
     * - @a arg is not a string, object, or array (or not a reference to a string / object / array)
     * - @a arg is an array but lacks the zeroth or the first element, or the zeroth element
     * is not an object, string, reference to an object / string, or the first element is not a string
     * or reference to a string
     */
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

    /**
     * @brief Destructor
     */
    ~Callable()
    {
        zval_dtor(&this->m_z);
#ifdef PHPCXX_DEBUG
        ZVAL_UNDEF(&this->m_z);
#endif
    }

    /**
     * @brief Move constructor
     * @param other Callable being moved
     */
    Callable(Callable&& other)
    {
        ZVAL_UNDEF(&this->m_z);
        std::swap(this->m_z, other.m_z);
    }

    /**
     * @brief Populates Function Call Information and Function Call Information Cache
     * @param[out] fci Function Call Information
     * @param[out] fcc Function Call Information Cache
     * @return Whether it was possible to resolve the callable
     * @retval false This typically means that the Zend Engine was unable to find the method / function
     * specified by the callable
     */
    bool resolve(zend_fcall_info& fci, zend_fcall_info_cache& fcc)
    {
        if (EXPECTED(Z_TYPE(this->m_z) != IS_UNDEF)) {
            int res = zend_fcall_info_init(&this->m_z, IS_CALLABLE_CHECK_SILENT, &fci, &fcc, nullptr, nullptr);
            return (SUCCESS == res);
        }

        std::memset(&fci, 0, sizeof(fci));
        std::memset(&fcc, 0, sizeof(fcc));
        return false;
    }

private:
    /**
     * @brief Internal representation of the callable
     */
    zval m_z;
};

}

#endif /* PHPCXX_CALLABLE_H */

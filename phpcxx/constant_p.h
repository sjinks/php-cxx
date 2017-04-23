#ifndef PHPCXX_CONSTANT_P_H
#define PHPCXX_CONSTANT_P_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_constants.h>
}

#include <cassert>
#include <cstring>

namespace phpcxx {

/**
 * @internal
 * @brief Implemenation details of phpcxx::Constant
 * @see Constant
 */
class PHPCXX_HIDDEN ConstantPrivate {
public:
    /**
     * @brief Creates a `NULL` constant
     * @param name Constant name
     */
    [[gnu::nonnull]] ConstantPrivate(const char* name)
    {
        assert(name != nullptr);
        ZVAL_NULL(&this->m_c.value);
        this->m_c.flags = CONST_CS | CONST_PERSISTENT;
        this->m_c.name  = zend_string_init(name, std::strlen(name), 1);
        this->m_c.module_number = -1;
    }

    /**
     * @brief Creates an integer constant
     * @param name Constant name
     * @param v Constant value
     */
    [[gnu::nonnull]] ConstantPrivate(const char* name, zend_long v)
    {
        assert(name != nullptr);
        ZVAL_LONG(&this->m_c.value, v);
        this->m_c.flags = CONST_CS | CONST_PERSISTENT;
        this->m_c.name  = zend_string_init(name, std::strlen(name), 1);
        this->m_c.module_number = -1;
    }

    /**
     * @brief Creates a double constant
     * @param name Constant name
     * @param v Constant value
     */
    [[gnu::nonnull]] ConstantPrivate(const char* name, double v)
    {
        assert(name != nullptr);
        ZVAL_DOUBLE(&this->m_c.value, v);
        this->m_c.flags = CONST_CS | CONST_PERSISTENT;
        this->m_c.name  = zend_string_init(name, std::strlen(name), 1);
        this->m_c.module_number = -1;
    }

    /**
     * @brief Creates a boolean constant
     * @param name Constant name
     * @param v Constant value
     */
    [[gnu::nonnull]] ConstantPrivate(const char* name, bool v)
    {
        assert(name != nullptr);
        ZVAL_BOOL(&this->m_c.value, v);
        this->m_c.flags = CONST_CS | CONST_PERSISTENT;
        this->m_c.name  = zend_string_init(name, std::strlen(name), 1);
        this->m_c.module_number = -1;
    }

    /**
     * @brief Creates a string constant
     * @param name Constant name
     * @param value Constant value
     * @param value_len Length of `value`
     */
    [[gnu::nonnull]] ConstantPrivate(const char* name, const char* value, std::size_t value_len)
    {
        assert(name != nullptr);
        assert(value != nullptr);
        ZVAL_NEW_STR(&this->m_c.value, zend_string_init(value, value_len, 1));
        this->m_c.flags = CONST_CS | CONST_PERSISTENT;
        this->m_c.name  = zend_string_init(name, std::strlen(name), 1);
        this->m_c.module_number = -1;
    }

    /**
     * @brief Class destructor
     *
     * Releases constant name and value if the constant
     * has not been registered with PHP
     *
     * @note Registered constants have a positive value of `module_number`
     */
    ~ConstantPrivate()
    {
        if (-1 == this->m_c.module_number) {
            zend_string_release(this->m_c.name);
            zval_ptr_dtor(&this->m_c.value);
        }
    }

    /**
     * @brief Returns the constant as a `zend_constant`
     * @return Value that can be used in the Zend API
     */
    zend_constant& get()
    {
        return this->m_c;
    }

    /**
     * @brief Registers the constant with the Zend Engine
     * @return Whether registration succeeded
     */
    bool registerConstant()
    {
        if (EG(current_module)) {
            zend_constant& zc = this->get();
            zc.module_number  = EG(current_module)->module_number;
            zend_string_addref(zc.name);
            if (FAILURE == zend_register_constant(&zc)) {
                // Zend calls zend_string_release() for constant name upon failure
                zc.module_number = -1;
                return false;
            }

            zend_string_release(zc.name);
            return true;
        }

        return false;
    }

private:
    /**
     * @brief Zend Constant
     */
    zend_constant m_c;
};

}

#endif /* PHPCXX_CONSTANT_P_H */

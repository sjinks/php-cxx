#ifndef PHPCXX_CONSTANT_P_H
#define PHPCXX_CONSTANT_P_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend_constants.h>
}

#include <cstring>

namespace phpcxx {

class PHPCXX_HIDDEN ConstantPrivate {
public:
    ConstantPrivate(const char* name)
    {
        ZVAL_NULL(&this->m_c.value);
        this->m_c.flags = CONST_CS | CONST_PERSISTENT;
        this->m_c.name  = zend_string_init(name, std::strlen(name), 1);
        this->m_c.module_number = -1;
    }

    ConstantPrivate(const char* name, zend_long v)
    {
        ZVAL_LONG(&this->m_c.value, v);
        this->m_c.flags = CONST_CS | CONST_PERSISTENT;
        this->m_c.name  = zend_string_init(name, std::strlen(name), 1);
        this->m_c.module_number = -1;
    }

    ConstantPrivate(const char* name, double v)
    {
        ZVAL_DOUBLE(&this->m_c.value, v);
        this->m_c.flags = CONST_CS | CONST_PERSISTENT;
        this->m_c.name  = zend_string_init(name, std::strlen(name), 1);
        this->m_c.module_number = -1;
    }

    ConstantPrivate(const char* name, bool v)
    {
        ZVAL_BOOL(&this->m_c.value, v);
        this->m_c.flags = CONST_CS | CONST_PERSISTENT;
        this->m_c.name  = zend_string_init(name, std::strlen(name), 1);
        this->m_c.module_number = -1;
    }

    ConstantPrivate(const char* name, const char* value, std::size_t value_len)
    {
        ZVAL_NEW_STR(&this->m_c.value, zend_string_init(value, value_len, 1));
        this->m_c.flags = CONST_CS | CONST_PERSISTENT;
        this->m_c.name  = zend_string_init(name, std::strlen(name), 1);
        this->m_c.module_number = -1;
    }

    ~ConstantPrivate()
    {
        if (-1 == this->m_c.module_number) {
            zend_string_release(this->m_c.name);
            zval_ptr_dtor(&this->m_c.value);
        }
    }

    zend_constant& get()
    {
        return this->m_c;
    }

private:
    zend_constant m_c;
};

}



#endif /* PHPCXX_CONSTANT_P_H */

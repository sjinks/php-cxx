#include <cstring>
#include "classconstant.h"

extern "C" {
#include <Zend/zend_API.h>
#include <Zend/zend_string.h>
#include <Zend/zend_types.h>
}

phpcxx::ClassConstant::ClassConstant(const char* name)
    : m_name(name)
{
    ZVAL_NULL(&this->m_value);
}

phpcxx::ClassConstant::ClassConstant(const char* name, zend_long v)
    : m_name(name)
{
    ZVAL_LONG(&this->m_value, v);
}

phpcxx::ClassConstant::ClassConstant(const char* name, double v)
    : m_name(name)
{
    ZVAL_DOUBLE(&this->m_value, v);
}

phpcxx::ClassConstant::ClassConstant(const char* name, bool v)
    : m_name(name)
{
    ZVAL_BOOL(&this->m_value, v);
}

phpcxx::ClassConstant::ClassConstant(const char* name, const std::string& v)
    : m_name(name)
{
    ZVAL_STRINGL(&this->m_value, v.c_str(), v.size());
}

phpcxx::ClassConstant::ClassConstant(const char* name, const char* v)
    : m_name(name)
{
    if (v && *v) {
        ZVAL_NEW_STR(&this->m_value, zend_string_init(name, std::strlen(name), 1));
    }
    else {
        ZVAL_NEW_STR(&this->m_value, zend_string_init("", 0, 1));
    }
}

phpcxx::ClassConstant::ClassConstant(phpcxx::ClassConstant&& other)
    : m_name(other.m_name)
{
    ZVAL_NULL(&this->m_value);
    std::swap(this->m_value, other.m_value);
}

phpcxx::ClassConstant::ClassConstant(const ClassConstant& other)
    : m_name(other.m_name), m_value(other.m_value)
{
}

phpcxx::ClassConstant::~ClassConstant()
{
    zval_dtor(&this->m_value);
}

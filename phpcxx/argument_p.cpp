#include <cstring>
#include "argument_p.h"

phpcxx::ArgumentPrivate::ArgumentPrivate(const char* argName, int type, const char* className, bool nullable, bool byRef, bool isVariadic)
    : m_class(nullptr)
{
    // argName can be nullptr; see ZEND_ARG_PASS_INFO macro
    this->m_arginfo.name              = argName;
    this->m_arginfo.pass_by_reference = byRef;
    this->m_arginfo.is_variadic       = isVariadic;
#if PHP_VERSION_ID < 70200
    this->m_arginfo.class_name        = className;
    this->m_arginfo.type_hint         = static_cast<zend_uchar>(type);
    this->m_arginfo.allow_null        = nullable;
#else
    if (className) {
        if (nullable) {
            delete[] this->m_class;
            this->m_class = new char[std::strlen(className)+2];
            this->m_class[0] = '?';
            std::memcpy(this->m_class + 1, className, std::strlen(className)+1);
            this->m_arginfo.type = reinterpret_cast<zend_type>(this->m_class);
        }
        else {
            this->m_arginfo.type = reinterpret_cast<zend_type>(className);
        }
    }
    else {
        this->m_arginfo.type = ZEND_TYPE_ENCODE(type, nullable);
    }
#endif
}

const char* phpcxx::ArgumentPrivate::name() const
{
    return this->m_arginfo.name;
}

const char* phpcxx::ArgumentPrivate::className() const
{
#if PHP_VERSION_ID < 70200
    return this->m_arginfo.class_name;
#else
    // See ZEND_TYPE_NAME, ZEND_TYPE_ALLOW_NULL macros
    zend_type type = this->m_arginfo.type;
    if (ZEND_TYPE_IS_CLASS(type)) {
        if ((type & 0x02) == 0x02) {
            zend_string* s  = ZEND_TYPE_NAME(type);
            const char* res = ZSTR_VAL(s);
            return res[0] == '?' ? res+1 : res;
        }

        return reinterpret_cast<const char*>(type);
    }

    return nullptr;
#endif
}

zend_uchar phpcxx::ArgumentPrivate::type() const
{
#if PHP_VERSION_ID < 70200
    int type_hint = this->m_arginfo.type_hint;
    if (IS_TRUE == type_hint || IS_FALSE == type_hint) {
        return _IS_BOOL;
    }

    return type_hint;
#else
    zend_type type = this->m_arginfo.type;
    if (ZEND_TYPE_IS_CODE(type)) {
        uintptr_t code = ZEND_TYPE_CODE(type);
        if (IS_TRUE == code || IS_FALSE == code) {
            return _IS_BOOL;
        }

        return ZEND_TYPE_CODE(type);
    }

    if (ZEND_TYPE_IS_CLASS(type)) {
        return IS_OBJECT;
    }

    return IS_UNDEF;
#endif
}

bool phpcxx::ArgumentPrivate::nullable() const
{
#if PHP_VERSION_ID < 70200
    return this->m_arginfo.allow_null;
#else
    // See ZEND_TYPE_NAME, ZEND_TYPE_ALLOW_NULL macros
    zend_type type = this->m_arginfo.type;
    if (ZEND_TYPE_IS_CLASS(type)) {
        if ((type & 0x02) == 0x02) {
            return ZEND_TYPE_ALLOW_NULL(type);
        }

        const char* name = reinterpret_cast<const char*>(type);
        return name[0] == '?';
    }

    return ZEND_TYPE_ALLOW_NULL(type);
#endif
}

bool phpcxx::ArgumentPrivate::isPassedByReference() const
{
    return this->m_arginfo.pass_by_reference;
}

bool phpcxx::ArgumentPrivate::isVariadic() const
{
    return this->m_arginfo.is_variadic;
}

void phpcxx::ArgumentPrivate::setType(zend_uchar type, bool nullable)
{
#if PHP_VERSION_ID < 70200
    this->m_arginfo.type_hint  = type;
    this->m_arginfo.allow_null = nullable;
#else
    this->m_arginfo.type = ZEND_TYPE_ENCODE(type, nullable);
#endif
}

void phpcxx::ArgumentPrivate::setClass(const char* name, bool nullable)
{
#if PHP_VERSION_ID < 70200
    this->m_arginfo.type_hint  = IS_OBJECT;
    this->m_arginfo.class_name = name;
    this->m_arginfo.allow_null = nullable;
#else
    if (nullable) {
        delete[] this->m_class;
        this->m_class = new char[std::strlen(name)+2];
        this->m_class[0] = '?';
        std::memcpy(this->m_class + 1, name, std::strlen(name)+1);
        this->m_arginfo.type = reinterpret_cast<zend_type>(this->m_class);
    }
    else {
        this->m_arginfo.type = reinterpret_cast<zend_type>(name);
    }
#endif
}

void phpcxx::ArgumentPrivate::setByRef(bool v)
{
    this->m_arginfo.pass_by_reference = v;
}

void phpcxx::ArgumentPrivate::setVariadic(bool v)
{
    this->m_arginfo.is_variadic = v;
}

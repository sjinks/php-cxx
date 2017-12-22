#include "argument.h"
#include "argument_p.h"
#include "value.h"

phpcxx::Argument::Argument(const char* argName)
    : d_ptr(new ArgumentPrivate(argName, IS_UNDEF, nullptr, false, false, false))
{
}

phpcxx::Argument::Argument(const char* argName, phpcxx::ArgumentType type, bool allowNull, bool byRef, bool isVariadic)
    : d_ptr(new ArgumentPrivate(argName, static_cast<int>(type), nullptr, allowNull, byRef, isVariadic))
{
}

phpcxx::Argument::Argument(const char* argName, const char* className, bool allowNull, bool byRef, bool isVariadic)
    : d_ptr(new ArgumentPrivate(argName, IS_OBJECT, className, allowNull, byRef, isVariadic))
{
}

phpcxx::Argument::Argument(const struct _zend_internal_arg_info& other)
    : d_ptr(new ArgumentPrivate(other))
{
}

phpcxx::Argument::Argument(phpcxx::Argument&& other) noexcept
    : d_ptr(std::move(other.d_ptr))
{
}

phpcxx::Argument::~Argument()
{
}

const char* phpcxx::Argument::name() const
{
    return this->d_ptr->m_arginfo.name;
}

const char* phpcxx::Argument::className() const
{
#if PHP_VERSION_ID < 70200
    return this->d_ptr->m_arginfo.class_name;
#else
    return ZEND_TYPE_IS_CLASS(this->d_ptr->m_arginfo.type) ? ZSTR_VAL(ZEND_TYPE_NAME(this->d_ptr->m_arginfo.type)) : nullptr;
#endif
}

phpcxx::ArgumentType phpcxx::Argument::type() const
{
#if PHP_VERSION_ID < 70200
    int type_hint = this->d_ptr->m_arginfo.type_hint;
    if (IS_TRUE == type_hint || IS_FALSE == type_hint) {
        return phpcxx::ArgumentType::Bool;
    }

    return static_cast<phpcxx::ArgumentType>(type_hint);
#else
    zend_type type = this->d_ptr->m_arginfo.type;
    if (ZEND_TYPE_IS_CODE(type)) {
        uintptr_t code = ZEND_TYPE_CODE(type);
        if (IS_TRUE == code || IS_FALSE == code) {
            return phpcxx::ArgumentType::Bool;
        }

        return static_cast<phpcxx::ArgumentType>(ZEND_TYPE_CODE(type));
    }

    if (ZEND_TYPE_IS_CLASS(type)) {
        return phpcxx::ArgumentType::Object;
    }

    return phpcxx::ArgumentType::Any;
#endif
}

bool phpcxx::Argument::canBeNull() const
{
#if PHP_VERSION_ID < 70200
    return this->d_ptr->m_arginfo.allow_null;
#else
    return ZEND_TYPE_ALLOW_NULL(this->d_ptr->m_arginfo.type);
#endif
}

bool phpcxx::Argument::isPassedByReference() const
{
    return this->d_ptr->m_arginfo.pass_by_reference;
}

bool phpcxx::Argument::isVariadic() const
{
    return this->d_ptr->m_arginfo.is_variadic;
}

phpcxx::Argument&& phpcxx::Argument::setType(phpcxx::ArgumentType type)
{
#if PHP_VERSION_ID < 70200
    this->d_ptr->m_arginfo.type_hint = static_cast<zend_uchar>(type);
#else
    this->d_ptr->m_arginfo.type = ZEND_TYPE_ENCODE(static_cast<zend_uchar>(type), ZEND_TYPE_ALLOW_NULL(this->d_ptr->m_arginfo.type));
#endif
    return std::move(*this);
}

phpcxx::Argument&& phpcxx::Argument::setClass(const char* name)
{
#if PHP_VERSION_ID < 70200
    this->d_ptr->m_arginfo.type_hint  = IS_OBJECT;
    this->d_ptr->m_arginfo.class_name = name;
#else
    this->d_ptr->m_arginfo.type       = ZEND_TYPE_ENCODE_CLASS(zend_string_init_interned(name, std::strlen(name), 1), ZEND_TYPE_ALLOW_NULL(this->d_ptr->m_arginfo.type));
#endif
    return std::move(*this);
}

phpcxx::Argument&& phpcxx::Argument::setNullable(bool v)
{
#if PHP_VERSION_ID < 70200
    this->d_ptr->m_arginfo.allow_null = v;
#else
    if (v) {
        this->d_ptr->m_arginfo.type |= 1;
    }
    else {
        this->d_ptr->m_arginfo.type &= ~1u;
    }
#endif
    return std::move(*this);
}

phpcxx::Argument&& phpcxx::Argument::setByRef(bool v)
{
    this->d_ptr->m_arginfo.pass_by_reference = v;
    return std::move(*this);
}

phpcxx::Argument&& phpcxx::Argument::setVariadic(bool v)
{
    this->d_ptr->m_arginfo.is_variadic = v;
    return std::move(*this);
}

const struct _zend_internal_arg_info& phpcxx::Argument::get() const
{
    return this->d_ptr->m_arginfo;
}

std::ostream& phpcxx::operator<<(std::ostream& os, const Argument& obj)
{
    os
        << "[Argument: name=" << obj.name()
        << ", type=" << Value::typeToString(static_cast<Type>(obj.type()))
        << ", nullable=" << obj.canBeNull()
        << ", byref=" << obj.isPassedByReference()
        << ", variadic=" << obj.isVariadic()
        << "]" << std::endl;
    ;

    return os;
}

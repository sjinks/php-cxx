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
    return this->d_ptr->m_arginfo.class_name;
}

phpcxx::ArgumentType phpcxx::Argument::type() const
{
    return static_cast<phpcxx::ArgumentType>(this->d_ptr->m_arginfo.type_hint);
}

bool phpcxx::Argument::canBeNull() const
{
    return this->d_ptr->m_arginfo.allow_null;
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
    this->d_ptr->m_arginfo.type_hint = static_cast<zend_uchar>(type);
    return std::move(*this);
}

phpcxx::Argument&& phpcxx::Argument::setClass(const char* name)
{
    this->d_ptr->m_arginfo.type_hint  = IS_OBJECT;
    this->d_ptr->m_arginfo.class_name = name;
    return std::move(*this);
}

phpcxx::Argument&& phpcxx::Argument::setNullable(bool v)
{
    this->d_ptr->m_arginfo.allow_null = v;
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

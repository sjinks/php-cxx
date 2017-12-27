#include <cstring>
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
    return this->d_ptr->name();
}

const char* phpcxx::Argument::className() const
{
    return this->d_ptr->className();
}

phpcxx::ArgumentType phpcxx::Argument::type() const
{
    return static_cast<phpcxx::ArgumentType>(this->d_ptr->type());
}

bool phpcxx::Argument::canBeNull() const
{
    return this->d_ptr->nullable();
}

bool phpcxx::Argument::isPassedByReference() const
{
    return this->d_ptr->isPassedByReference();
}

bool phpcxx::Argument::isVariadic() const
{
    return this->d_ptr->isVariadic();
}

phpcxx::Argument&& phpcxx::Argument::setType(phpcxx::ArgumentType type, bool nullable)
{
    this->d_ptr->setType(static_cast<zend_uchar>(type), nullable);
    return std::move(*this);
}

phpcxx::Argument&& phpcxx::Argument::setClass(const char* name, bool nullable)
{
    this->d_ptr->setClass(name, nullable);
    return std::move(*this);
}

phpcxx::Argument&& phpcxx::Argument::setByRef(bool v)
{
    this->d_ptr->setByRef(v);
    return std::move(*this);
}

phpcxx::Argument&& phpcxx::Argument::setVariadic(bool v)
{
    this->d_ptr->setVariadic(v);
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

#include <Zend/zend_types.h>
#include "argument.h"
#include "argument_p.h"
#include "value.h"

phpcxx::Argument::Argument(const char* argName, phpcxx::ArgumentType type, bool allowNull, bool byRef, bool isVariadic)
    : d_ptr(std::make_shared<ArgumentPrivate>(argName, static_cast<int>(type), nullptr, allowNull, byRef, isVariadic))
{
}

phpcxx::Argument::Argument(const char* argName, const char* className, bool allowNull, bool byRef, bool isVariadic)
    : d_ptr(std::make_shared<ArgumentPrivate>(argName, IS_OBJECT, className, allowNull, byRef, isVariadic))
{
}

phpcxx::Argument::Argument(const struct _zend_internal_arg_info& other)
    : d_ptr(std::make_shared<ArgumentPrivate>(other))
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

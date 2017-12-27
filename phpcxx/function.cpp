#include "function.h"
#include "function_p.h"
#include "value.h"

phpcxx::Function::Function(const char* name, InternalFunction c, std::size_t nreq, const Arguments& args, bool byRef)
    : d_ptr(std::make_shared<FunctionPrivate>(name, c, nreq, args, byRef))
{
}

phpcxx::Function::Function(const Function& other)
    : d_ptr(other.d_ptr)
{
}

phpcxx::Function::~Function()
{
}

phpcxx::Function&& phpcxx::Function::addArgument(const Argument& arg)
{
    auto& args = this->d_ptr->m_arginfo;
    args.push_back(arg.get());
    ++this->d_ptr->m_fe.num_args;
    this->d_ptr->m_fe.arg_info = args.data();
    return std::move(*this);
}

phpcxx::Function&& phpcxx::Function::setNumberOfRequiredArguments(std::size_t n)
{
    auto& args   = this->d_ptr->m_arginfo;
    args[0].name = reinterpret_cast<const char*>(static_cast<zend_intptr_t>(n));
    return std::move(*this);
}

phpcxx::Function&& phpcxx::Function::setReturnByReference(bool byref)
{
    this->d_ptr->m_arginfo[0].pass_by_reference = byref;
    return std::move(*this);
}

phpcxx::Function&& phpcxx::Function::setTypeHint(ArgumentType t, bool nullable)
{
    zend_internal_arg_info& info = this->d_ptr->m_arginfo[0];
#if PHP_VERSION_ID < 70200
    info.type_hint  = static_cast<zend_uchar>(t);
    info.allow_null = nullable;
#else
    info.type       = ZEND_TYPE_ENCODE(static_cast<zend_uchar>(t), nullable);
#endif
    return std::move(*this);
}

phpcxx::Function&& phpcxx::Function::setTypeHint(const char* className, bool nullable)
{
    zend_internal_arg_info& info = this->d_ptr->m_arginfo[0];
#if PHP_VERSION_ID < 70200
    info.type_hint  = IS_OBJECT;
    info.class_name = className;
    info.allow_null = nullable;
#else
    info.type       = ZEND_TYPE_ENCODE_CLASS(className, nullable);
#endif
    return std::move(*this);
}

phpcxx::Function&& phpcxx::Function::setDeprecated(bool v)
{
    if (v) {
        this->d_ptr->m_fe.flags |= ZEND_ACC_DEPRECATED;
    }
    else {
        this->d_ptr->m_fe.flags &= ~ZEND_ACC_DEPRECATED;
    }

    return std::move(*this);
}

const struct _zend_function_entry& phpcxx::Function::getFE() const
{
    return this->d_ptr->m_fe;
}

const std::vector<struct _zend_internal_arg_info>& phpcxx::Function::getArgInfo() const
{
    return this->d_ptr->m_arginfo;
}

std::vector<char*>& phpcxx::Function::getClassNames() const
{
    return this->d_ptr->m_classes;
}

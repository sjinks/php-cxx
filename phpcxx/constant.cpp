#include <cstring>
#include "constant.h"
#include "constant_p.h"

phpcxx::Constant::Constant(const char* name)
    : d_ptr(std::make_shared<ConstantPrivate>(name))
{
}

phpcxx::Constant::Constant(const char* name, zend_long v)
    : d_ptr(std::make_shared<ConstantPrivate>(name, v))
{
}

phpcxx::Constant::Constant(const char* name, double v)
    : d_ptr(std::make_shared<ConstantPrivate>(name, v))
{
}

phpcxx::Constant::Constant(const char* name, bool v)
    : d_ptr(std::make_shared<ConstantPrivate>(name, v))
{
}

phpcxx::Constant::Constant(const char* name, const std::string& v)
    : d_ptr(std::make_shared<ConstantPrivate>(name, v.c_str(), v.size()))
{
}

phpcxx::Constant::Constant(const char* name, const char* v)
    : d_ptr(std::make_shared<ConstantPrivate>(name, v, std::strlen(v)))
{
}

phpcxx::Constant::~Constant()
{
}

struct _zend_constant& phpcxx::Constant::get()
{
    return this->d_ptr->get();
}

bool phpcxx::Constant::registerConstant()
{
    return this->d_ptr->registerConstant();
}

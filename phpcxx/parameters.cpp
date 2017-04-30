#include "parameters.h"
#include "parameters_p.h"
#include "value.h"

phpcxx::Parameters::Parameters(std::initializer_list<Value*> l)
    : d_ptr(emcreate<ParametersPrivate>(l), emdeleter())
{
}

phpcxx::Parameters::Parameters(const vector<Value*>& v)
    : d_ptr(emcreate<ParametersPrivate>(v), emdeleter())
{
}

phpcxx::Parameters::Parameters()
    : d_ptr(emcreate<ParametersPrivate>(), emdeleter())
{
}

phpcxx::Parameters::~Parameters()
{
}

std::size_t phpcxx::Parameters::size() const
{
    return this->d_ptr->m_params.size();
}

bool phpcxx::Parameters::verify() const
{
    return this->d_ptr->verify();
}

phpcxx::Value& phpcxx::Parameters::operator[](std::size_t idx) const
{
    zval* z = this->d_ptr->m_params[idx];
    return *new(reinterpret_cast<void*>(z)) Value(placement_construct);
}

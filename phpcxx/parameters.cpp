#include <cassert>
#include <memory>
#include "parameters.h"
#include "parameters_p.h"
#include "emallocallocator.h"
#include "value.h"

phpcxx::Parameters::Parameters(std::initializer_list<phpcxx::Value> v)
    : d_ptr(emcreate<ParametersPrivate>(v), emdeleter())
{
}

phpcxx::Parameters::Parameters(ParametersPrivate* dd)
    : d_ptr(dd)
{
    assert(dd != nullptr);
}

phpcxx::Parameters::Parameters(int argc)
    : d_ptr(emcreate<ParametersPrivate>(argc), emdeleter())
{
}

phpcxx::Parameters::~Parameters()
{
}

std::size_t phpcxx::Parameters::count() const
{
    return this->d_ptr->m_params.size();
}

phpcxx::Value& phpcxx::Parameters::operator[](std::size_t idx)
{
    return this->d_ptr->m_params[idx];
}

const phpcxx::Value& phpcxx::Parameters::operator[](std::size_t idx) const
{
    return this->d_ptr->m_params[idx];
}

#include <cassert>
#include <memory>
#include "parameters.h"
#include "parameters_p.h"
#include "emallocallocator.h"
#include "value.h"

phpcxx::Parameters::Parameters(ParametersPrivate* dd)
    : d_ptr(dd)
{
    assert(dd != nullptr);
}

phpcxx::Parameters::Parameters()
    : d_ptr(emcreate<ParametersPrivate>(), emdeleter())
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
    FTRACE();
    zval* z = this->d_ptr->m_params[idx];
    return *new(reinterpret_cast<void*>(z)) Value(placement_construct);
}

const phpcxx::Value& phpcxx::Parameters::operator[](std::size_t idx) const
{
    FTRACE();
    zval* z = this->d_ptr->m_params[idx];
    return *new(reinterpret_cast<void*>(z)) Value(placement_construct);
}

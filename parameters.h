#ifndef PHPCXX_PARAMETERS_H
#define PHPCXX_PARAMETERS_H

#include "phpcxx.h"

#include <initializer_list>
#include <memory>

namespace phpcxx {

class Value;
class ParametersPrivate;
class PHPCXX_EXPORT Parameters {
public:
    Parameters(std::initializer_list<Value> v);
    Parameters(ParametersPrivate* dd);
    ~Parameters();

    std::size_t count() const;

    Value& operator[](std::size_t idx);
    const Value& operator[](std::size_t idx) const;

private:
    std::unique_ptr<ParametersPrivate> d_ptr;

    Parameters(int);
    friend class FunctionHandler;
};

}



#endif /* PARAMETERS_H_ */

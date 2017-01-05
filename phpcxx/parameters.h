#ifndef PHPCXX_PARAMETERS_H
#define PHPCXX_PARAMETERS_H

#include "phpcxx.h"

#include <initializer_list>
#include <memory>
#include "emallocallocator.h"
#include "vector.h"

struct _zend_execute_data;

namespace phpcxx {

class Value;
class ParametersPrivate;
class PHPCXX_EXPORT Parameters {
public:
    Parameters(std::initializer_list<Value*> l);
    Parameters(const vector<Value*>& v);
    ~Parameters();

    std::size_t size() const;

    bool verify() const;

    Value& operator[](std::size_t idx);
    const Value& operator[](std::size_t idx) const;

private:
    std::unique_ptr<ParametersPrivate, emdeleter> d_ptr;

    Parameters(struct _zend_execute_data* execute_data);
    friend class FunctionHandler;
};

}



#endif /* PARAMETERS_H_ */

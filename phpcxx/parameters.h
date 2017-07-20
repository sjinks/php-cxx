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

/**
 * @brief Class representing parameters passed to a function or a method
 */
class PHPCXX_EXPORT Parameters {
public:
    /**
     * @brief Default constructor
     * @details Constructs function/method parameters from `EG(current_execute_data)`
     */
    Parameters();

    /**
     * @brief Constructs parameters from `std::initializer_list`
     * @param l Parameters
     */
    Parameters(const std::initializer_list<Value*>& l);

    /**
     * @brief Constructs parameters from `phpcxx::vector`
     * @param v Parameters
     */
    Parameters(const vector<Value*>& v);

    /**
     * @brief Destructor
     */
    ~Parameters();

    /**
     * @brief Return the number of parameters passed
     * @return Number of parameters
     */
    std::size_t size() const;

    /**
     * @brief Performs parameter type / count check
     * @return Whether the check succeeded
     * @throws phpcxx::PhpException if Zend API functions throw an exception
     * @warning It makes sense to call this function only if `Parameters` class
     * has been constructed with the default constructor (i.e., from `EG(current_execute_data)`)
     * because the checks are performed for `EG(current_execute_data)`.
     */
    bool verify() const;

    /**
     * @brief Returns a parameter by its index
     * @param idx Parameter index
     * @return Parameter
     * @warning No bounds checking is performed
     */
    Value& operator[](std::size_t idx) const;

private:
    /**
     * @internal
     * @brief Implementation details
     */
    std::unique_ptr<ParametersPrivate, emdeleter> d_ptr;
};

}

#endif /* PHPCXX_PARAMETERS_H */

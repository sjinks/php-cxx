#ifndef PHPCXX_CALLABLE_H
#define PHPCXX_CALLABLE_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend.h>
}

namespace phpcxx {

/**
 * @brief Callable
 * @see http://php.net/manual/en/language.types.callable.php
 */
class PHPCXX_EXPORT Callable {
public:
    /**
     * @brief Constructs a new callable from a string
     * @param name Callable (function name or static class method)
     */
    [[gnu::nonnull]] Callable(const char* name);

    /**
     * @brief Constructs a new callable (static class method)
     * @param cls Class name
     * @param name Method name
     */
    [[gnu::nonnull]] Callable(const char* cls, const char* name);

    /**
     * @brief Constructs a new callable (possibly class method)
     * @param obj Class instance
     * @param name Method name
     * @throws std::invalid_argument if @a obj is neither (a reference to) an object nor (a reference to) a string
     */
    [[gnu::nonnull]] Callable(zval* obj, const char* name);

    /**
     * @brief Constructs a new callable from `zval*`
     * @param arg Callable
     * @throws std::invalid_argument if @a arg is not a valid callable
     * - @a arg is not a string, object, or array (or not a reference to a string / object / array)
     * - @a arg is an array but lacks the zeroth or the first element, or the zeroth element
     * is not an object, string, reference to an object / string, or the first element is not a string
     * or reference to a string
     */
    [[gnu::nonnull]] Callable(zval* arg);

    /**
     * @brief Destructor
     */
    ~Callable();

    /**
     * @brief Move constructor
     * @param other Callable being moved
     */
    Callable(Callable&& other);

    /**
     * @brief Populates Function Call Information and Function Call Information Cache
     * @param[out] fci Function Call Information
     * @param[out] fcc Function Call Information Cache
     * @return Whether it was possible to resolve the callable
     * @retval false This typically means that the Zend Engine was unable to find the method / function
     * specified by the callable
     * @note When `resolve()` returns `false`, the following is true: `fcc.size == 0 && fci.initialized == 0`
     */
    bool resolve(zend_fcall_info& fci, zend_fcall_info_cache& fcc);

private:
    /**
     * @brief Internal representation of the callable
     */
    zval m_z;
};

}

static_assert(sizeof(zval) == sizeof(phpcxx::Callable), "sizeof(zval) must be equal to sizeof(phpcxx::Callable)");

#endif /* PHPCXX_CALLABLE_H */

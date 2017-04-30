#ifndef PHPCXX_FCALL_H
#define PHPCXX_FCALL_H

#include "phpcxx.h"
#include "vector.h"
#include "helpers.h"
#include "callable.h"

extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_API.h>
}

namespace phpcxx {

class Value;

/**
 * @brief Function/method call
 */
class PHPCXX_EXPORT FCall {
public:
    /**
     * @brief Constructs FCall from low-level Zend API structures
     * @param fci Function call information
     * @param fcc Function call information cache
     * @throws std::invalid_argument `fci.size != sizeof(zend_fcall_info)`
     */
    FCall(const zend_fcall_info& fci, const zend_fcall_info_cache& fcc);

    /**
     * @brief Constructs FCall from a callable
     * @param c Callable
     * @throws std::invalid_argument Callable failed to resolve
     */
    FCall(Callable c);

    /**
     * @brief Constructs FCall from a function/method name
     * @param name Function (`func`) or static method (`class::method`) name
     * @see http://php.net/manual/en/language.types.callable.php
     * @throws std::invalid_argument Callable failed to resolve
     */
    [[gnu::nonnull]] FCall(const char* name)
        : FCall(Callable(name))
    {
    }

    /**
     * @brief Constructs FCall from a static method name
     * @param cls Class name
     * @param name Static method name
     * @see http://php.net/manual/en/language.types.callable.php
     * @throws std::invalid_argument Callable failed to resolve
     */
    [[gnu::nonnull]] FCall(const char* cls, const char* name)
        : FCall(Callable(cls, name))
    {
    }

    /**
     * @brief Constructs FCall from a (static) method name
     * @param obj Class instance (must be `IS_OBJECT`) or name (`IS_STRING`)
     * @param name Method name
     * @see http://php.net/manual/en/language.types.callable.php
     * @throws std::invalid_argument if @a obj is neither (a reference to) an object nor (a reference to) a string
     * @throws std::invalid_argument Callable failed to resolve
     */
    [[gnu::nonnull]] FCall(zval* obj, const char* name)
        : FCall(Callable(obj, name))
    {
    }

    /**
     * @brief Constructs FCall from @a arg
     * @param arg Callable `zval`
     * @see http://php.net/manual/en/language.types.callable.php
     * @throws std::invalid_argument if @a arg is not a valid callable
     * @throws std::invalid_argument Callable failed to resolve
     */
    [[gnu::nonnull]] FCall(zval* arg)
        : FCall(Callable(arg))
    {
    }

    /**
     * @brief Destructor
     */
    ~FCall();

    /**
     * @brief Invokes the callable passing @a args as arguments
     * @param args Arguments
     * @return Value returned by the callable
     * @throws std::runtime_error Call failed (`zend_call_function()` returned `FAILURE`)
     * @throws phpcxx::PhpException PHP exception occurred
     */
    Value operator()(vector<Value> args);

    /**
     * @overload
     */
    Value operator()(vector<zval> args);

    /**
     * @brief Invokes the callable passing @a p as arguments
     * @tparam Params Parameter pack; `Params` must be convertible to `zval`
     * @param p Arguments
     * @return Value returned by the callable
     * @throws std::runtime_error Call failed (`zend_call_function()` returned `FAILURE`)
     * @throws phpcxx::PhpException PHP exception occurred
     */
    template<typename... Params>
    Value operator()(Params&&... p);

private:
    /**
     * @name Implementation details
     */
    /**@{*/
    zend_fcall_info m_fci;          ///< Function call information
    zend_fcall_info_cache m_fcc;    ///< Function call information cache
    /**@}*/

    /**
     * @brief Invokes the callable passing @a n @a args as arguments
     * @param n Number of arguments
     * @param args Arguments
     * @return Value returned by the callable
     * @throws std::runtime_error Call failed (`zend_call_function()` returned `FAILURE`)
     * @throws phpcxx::PhpException PHP exception occurred
     */
    Value operator()(std::size_t n, zval* args);

    /**
     * @brief Converts @a v into `zval`
     * @tparam T Type of @a v
     * @param[in] v Value to convert
     * @param[out] z Resulting `zval`
     * @return Pointer to @a z
     */
    template<typename T>
    [[gnu::returns_nonnull]] static zval* paramHelper(T&& v, zval& z) { construct_zval(z, v); Z_TRY_DELREF(z); return &z; }

    /**
     * @brief Helper function
     * @tparam Args Types of the arguments
     * @tparam Is Indices of the arguments
     * @param args Arguments
     * @return Call result
     */
    template<typename ...Args, std::size_t ...Is>
    Value call(indices<Is...>, Args&&... args);

};

/**
 * @brief Invokes callable @a name passing @a p as arguments
 * @tparam Params Parameter pack; `Params` must be convertible to `zval`
 * @param name Callable
 * @param p Arguments
 * @return Value returned by the callable
 * @throws std::invalid_argument Callable failed to resolve
 * @throws std::runtime_error Call failed (`zend_call_function()` returned `FAILURE`)
 * @throws phpcxx::PhpException PHP exception occurred
 */
template<typename... Params> static phpcxx::Value call(const char* name, Params&&... p);

/**
 * @brief Invokes callable @a v passing @a p as arguments
 * @tparam Params Parameter pack; `Params` must be convertible to `zval`
 * @param v Callable
 * @param p Arguments
 * @return Value returned by the callable
 * @throws std::invalid_argument Callable failed to resolve
 * @throws std::runtime_error Call failed (`zend_call_function()` returned `FAILURE`)
 * @throws phpcxx::PhpException PHP exception occurred
 */
template<typename... Params> static phpcxx::Value call(const phpcxx::Value& v, Params&&... p);

}

#include "fcall.tcc"

#endif /* PHPCXX_FCALL_H */

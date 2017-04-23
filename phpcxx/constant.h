#ifndef PHPCXX_CONSTANT_H
#define PHPCXX_CONSTANT_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend_long.h>
}

#include <memory>
#include <type_traits>

struct _zend_constant;

namespace phpcxx {

class ConstantPrivate;

/**
 * @brief Wrapper class around @c zend_constant
 */
class PHPCXX_EXPORT Constant {
public:
    /**
     * @brief Creates a `NULL` constant
     * @param name Constant name
     */
    [[gnu::nonnull]] Constant(const char* name);

    /**
     * @brief Creates an integer constant
     * @param name Constant name
     * @param v Constant value
     */
    [[gnu::nonnull]] Constant(const char* name, zend_long v);

    /**
     * @brief Creates a double constant
     * @param name Constant name
     * @param v Constant value
     */
    [[gnu::nonnull]] Constant(const char* name, double v);

    /**
     * @brief Creates a boolean constant
     * @param name Constant name
     * @param v Constant value
     */
    [[gnu::nonnull]] Constant(const char* name, bool v);

    /**
     * @brief Creates a string constant
     * @param name Constant name
     * @param v Constant value
     */
    [[gnu::nonnull]] Constant(const char* name, const std::string& v);

    /**
     * @brief Creates a string constant
     * @param name Constant name
     * @param v Constant value
     */
    [[gnu::nonnull]] Constant(const char* name, const char* v);

    /**
     * @brief Class destructor
     */
    ~Constant();

    /**
     * @brief Copy constructor
     */
    Constant(const Constant& other) : d_ptr(other.d_ptr) {}

    /**
     * @brief Creates an integer constant
     * @param name Constant name
     * @param v Constant value
     *
     * This template is used for the cases when @c sizeof(T) (typically @c int) is less than @c sizeof(zend_long),
     * where @c T is of an integral type
     */
    template<typename T, typename std::enable_if<(sizeof(T) < sizeof(zend_long)) && std::is_integral<T>::value>::type* = nullptr>
    [[gnu::nonnull]] Constant(const char* name, T v) : Constant(name, static_cast<zend_long>(v)) {}

    /**
     * @brief Returns the constructed constant which can be used with the Zend API
     * @return Value that can be used in the Zend API
     */
    struct _zend_constant& get();

    /**
     * @brief Registers the constant with the Zend Engine
     * @return Whether registration succeeded
     * @note Registration fails if:
     * - another constant with the same name is already registered,
     * - registration happens after @c MINIT phase
     */
    bool registerConstant();
private:
    /**
     * @internal
     * @brief Opaque pointer to implementation details
     */
    std::shared_ptr<ConstantPrivate> d_ptr;
};

}

#endif /* PHPCXX_CONSTANT_H */

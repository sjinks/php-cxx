#ifndef PHPCXX_TYPETRAITS_H
#define PHPCXX_TYPETRAITS_H

#include <type_traits>

namespace phpcxx {

/**
 * @defgroup helpertypes Helper types from C++14
 * @{
 */
#if __cplusplus < 201402L
template<typename T>                using remove_cv_t        = typename std::remove_cv<T>::type;
template<typename T>                using remove_pointer_t   = typename std::remove_pointer<T>::type;
template<typename T>                using remove_extent_t    = typename std::remove_extent<T>::type;
template<bool B, typename T = void> using enable_if_t        = typename std::enable_if<B, T>::type;
#else
template<typename T>                using remove_cv_t        = std::remove_cv_t<T>;
template<typename T>                using remove_pointer_t   = std::remove_pointer_t<T>;
template<typename T>                using remove_extent_t    = std::remove_extent_t<T>;
template<bool B, typename T = void> using enable_if_t        = std::enable_if_t<B, T>;
#endif
/**
 * @}
 */

/**
 * @defgroup typetraits Type traits
 * @{
 */
#if __cplusplus < 201402L
/** @internal */
template<typename>   struct is_null_pointer_helper : public std::false_type {};
/** @internal */
template<>           struct is_null_pointer_helper<std::nullptr_t> : public std::true_type {};
/**
 * @brief Checks whether @a T is of `std::nullptr_t` type
 * @tparam T Type to check
 */
template<typename T> struct is_null_pointer : public is_null_pointer_helper<remove_cv_t<T> >::type {};
#else
template<typename T> struct is_null_pointer : public std::is_null_pointer<T> {};
#endif

/**
 * @brief Checks whether @a T is `bool`
 * @tparam T Type to check
 */
template<typename T> struct is_bool    : public std::integral_constant<bool, std::is_same<T, bool>::value> {};

/**
 * @brief Checks whether @a T is of integral type AND is not `bool`
 * @tparam T Type to check
 */
template<typename T> struct is_integer : public std::integral_constant<bool, std::is_integral<T>::value && !std::is_same<T, bool>::value> {};

/**
 * @brief Checks whether @a T is of numeric type (integral or floating point)
 * @tparam T Type to check
 */
template<typename T> struct is_numeric : public std::integral_constant<bool, is_integer<T>::value || std::is_floating_point<T>::value> {};

/**
 * @brief Checks whether @a T is `zval*`
 * @tparam T Type to check
 */
template<typename T> struct is_pzval   : public std::integral_constant<bool, std::is_same<zval*, T>::value> {};

/**
 * @brief Checks whether @a T with cv-qualifiers removed is @ref phpcxx::string
 * @tparam T Type to check
 */
template<typename T> struct is_string  : public std::integral_constant<bool, std::is_same<string, remove_cv_t<T> >::value> {};

/**
 * @brief Check whether @a T with cv-qualifiers removed is `char*` or `char[]`
 * @tparam T Type to check
 */
template<typename T>
struct is_pchar : public
    std::integral_constant<
        bool,
           std::is_same<char*, remove_cv_t<T> >::value
        || (   std::is_pointer<T>::value
            && std::is_same<char, remove_cv_t<remove_pointer_t<remove_cv_t<T> > > >::value
           )
        || std::is_same<char, remove_cv_t<remove_extent_t<T> > >::value
    >
{};
/**
 * @}
 */

}

#endif /* PHPCXX_TYPETRAITS_H */

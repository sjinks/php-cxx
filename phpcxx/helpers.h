#ifndef PHPCXX_HELPERS_H
#define PHPCXX_HELPERS_H

#include "phpcxx.h"

#include <cstring>
#include <type_traits>
#include "string.h"
#include "zendstring.h"

namespace phpcxx {

struct placement_construct_t {
    explicit placement_construct_t() {}
};

extern const placement_construct_t placement_construct;

// See http://loungecpp.wikidot.com/tips-and-tricks:indices
template<std::size_t... Is>
struct indices {};

template<std::size_t N, std::size_t... Is>
struct build_indices : build_indices<N-1, N-1, Is...> {};

template<std::size_t... Is>
struct build_indices<0, Is...> : indices<Is...> {};

template<typename ...T>
using IndicesFor = build_indices<sizeof...(T)>;

/**
 * @defgroup Helper types from C++14
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
 * @defgroup is_null_pointer
 * @{
 */
#if __cplusplus < 201402L
template<typename>   struct is_null_pointer_helper : public std::false_type {};
template<>           struct is_null_pointer_helper<std::nullptr_t> : public std::true_type {};
template<typename T> struct is_null_pointer : public is_null_pointer_helper<remove_cv_t<T> >::type {};
#else
template<typename T> struct is_null_pointer : public std::is_null_pointer<T> {};
#endif
/**
 * @}
 */

template<typename T> struct is_bool    : public std::integral_constant<bool, std::is_same<T, bool>::value> {};
template<typename T> struct is_integer : public std::integral_constant<bool, std::is_integral<T>::value && !std::is_same<T, bool>::value> {};
template<typename T> struct is_numeric : public std::integral_constant<bool, is_integer<T>::value || std::is_floating_point<T>::value> {};
template<typename T> struct is_pzval   : public std::integral_constant<bool, std::is_same<zval*, T>::value> {};
template<typename T> struct is_string  : public std::integral_constant<bool, std::is_same<string, remove_cv_t<T> >::value> {};

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
 * @defgroup Construction of zvals
 * @{
 */
template<typename T, enable_if_t<is_null_pointer<T>::value>* = nullptr>
void construct_zval(zval& z, T)   { ZVAL_NULL(&z); }

template<typename T, enable_if_t<is_bool<T>::value>* = nullptr>
void construct_zval(zval& z, T v) { ZVAL_BOOL(&z, v); }

template<typename T, enable_if_t<is_integer<T>::value>* = nullptr>
void construct_zval(zval& z, T v) { ZVAL_LONG(&z, v); }

template<typename T, enable_if_t<std::is_floating_point<T>::value>* = nullptr>
void construct_zval(zval& z, T v) { ZVAL_DOUBLE(&z, v); }

template<typename T, enable_if_t<is_pchar<T>::value || is_string<T>::value>* = nullptr>
void construct_zval(zval& z, T s)
{
    ZendString zs(s);
    ZVAL_STR(&z, zs.release());
}
/**
 * @}
 */

/**
 * @defgroup Assignment
 * @{
 */

/**
 * Assign a null/boolean/integer/double to a zval
 * @param a
 * @param b
 */
template<typename T, enable_if_t<std::is_arithmetic<T>::value || is_null_pointer<T>::value>* = nullptr>
void assign(zval* a, T b)
{
    if (Z_ISREF_P(a)) {
        a = Z_REFVAL_P(a);
    }

    if (UNEXPECTED(Z_IMMUTABLE_P(a))) {
        zend_error(E_ERROR, "Cannot assign to an immutable variable");
        return;
    }

    if (Z_REFCOUNTED_P(a)) {
        if (Z_TYPE_P(a) == IS_OBJECT && UNEXPECTED(Z_OBJ_HANDLER_P(a, set))) {
            zval tmp;
            construct_zval(tmp, b);
            assert(!Z_OPT_REFCOUNTED(tmp));
            Z_OBJ_HANDLER_P(a, set)(a, &tmp);
            return;
        }

        zend_refcounted* r = Z_COUNTED_P(a);
        if (--GC_REFCOUNT(r) == 0) {
#if PHP_VERSION_ID >= 70100
            zval_dtor_func(r);
#else
            zval_dtor_func_for_ptr(r);
#endif
        }
        else {
            if (Z_COLLECTABLE_P(a) && UNEXPECTED(!GC_INFO(r))) {
                gc_possible_root(r);
            }
        }
    }

    phpcxx::construct_zval(*a, b);
}

/**
 * Assign a string/char* to zval
 * @param a
 * @param b
 */
template<typename T, enable_if_t<is_string<T>::value || is_pchar<T>::value>* = nullptr>
void assign(zval* a, const T& b)
{
    if (Z_ISREF_P(a)) {
        a = Z_REFVAL_P(a);
    }

    if (UNEXPECTED(Z_IMMUTABLE_P(a))) {
        zend_error(E_ERROR, "Cannot assign to an immutable variable");
        return;
    }

    if (Z_REFCOUNTED_P(a)) {
        if (Z_TYPE_P(a) == IS_OBJECT && UNEXPECTED(Z_OBJ_HANDLER_P(a, set))) {
            zval tmp;
            phpcxx::construct_zval(tmp, b);
            Z_OBJ_HANDLER_P(a, set)(a, &tmp);
            i_zval_ptr_dtor(&tmp ZEND_FILE_LINE_CC);
            return;
        }

        zend_refcounted* r = Z_COUNTED_P(a);
        if (--GC_REFCOUNT(r) == 0) {
#if PHP_VERSION_ID >= 70100
            zval_dtor_func(r);
#else
            zval_dtor_func_for_ptr(r);
#endif
        }
        else {
            if (Z_COLLECTABLE_P(a) && UNEXPECTED(!GC_INFO(r))) {
                gc_possible_root(r);
            }
        }
    }

    phpcxx::construct_zval(*a, b);
}

/**
 * Assign zval to zval
 * @param a
 * @param b
 */
template<typename T, enable_if_t<is_pzval<T>::value>* = nullptr>
static void assign(zval* a, T b)
{
    if (Z_ISREF_P(b)) {
        b = Z_REFVAL_P(b);
    }

    if (Z_ISREF_P(a)) {
        a = Z_REFVAL_P(a);
    }

    if (UNEXPECTED(Z_IMMUTABLE_P(a))) {
        zend_error(E_ERROR, "Cannot assign to an immutable variable");
        return;
    }

    if (Z_REFCOUNTED_P(a)) {
        if (Z_TYPE_P(a) == IS_OBJECT && UNEXPECTED(Z_OBJ_HANDLER_P(a, set))) {
            Z_OBJ_HANDLER_P(a, set)(a, b);
            return;
        }

        if (a == b) {
            return;
        }

        zend_refcounted* r = Z_COUNTED_P(a);
        if (--GC_REFCOUNT(r) == 0) {
#if PHP_VERSION_ID >= 70100
            zval_dtor_func(r);
#else
            zval_dtor_func_for_ptr(r);
#endif
        }
        else {
            if (Z_COLLECTABLE_P(a) && UNEXPECTED(!GC_INFO(r))) {
                gc_possible_root(r);
            }
        }
    }

    ZVAL_COPY(a, b);
}

static inline void assignTemporary(zval* a, zval* b)
{
    if (Z_ISREF_P(b)) {
        b = Z_REFVAL_P(b);
    }

    if (Z_ISREF_P(a)) {
        a = Z_REFVAL_P(a);
    }

    if (UNEXPECTED(Z_IMMUTABLE_P(a))) {
        zend_error(E_ERROR, "Cannot assign to an immutable variable");
        return;
    }

    if (Z_REFCOUNTED_P(a)) {
        if (Z_TYPE_P(a) == IS_OBJECT && UNEXPECTED(Z_OBJ_HANDLER_P(a, set))) {
            Z_OBJ_HANDLER_P(a, set)(a, b);
            i_zval_ptr_dtor(b ZEND_FILE_LINE_CC);
            return;
        }

        if (a == b) {
            return;
        }

        zend_refcounted* r = Z_COUNTED_P(a);
        if (--GC_REFCOUNT(r) == 0) {
#if PHP_VERSION_ID >= 70100
            zval_dtor_func(r);
#else
            zval_dtor_func_for_ptr(r);
#endif
        }
        else {
            if (Z_COLLECTABLE_P(a) && UNEXPECTED(!GC_INFO(r))) {
                gc_possible_root(r);
            }
        }
    }

    ZVAL_COPY_VALUE(a, b);
}
/**
 * @}
 */
}

#endif /* PHPCXX_HELPERS_H */

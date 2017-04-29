#ifndef PHPCXX_HELPERS_H
#define PHPCXX_HELPERS_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend.h>
}

#include <cstring>
#include "string.h"
#include "typetraits.h"
#include "zendstring.h"

namespace phpcxx {

struct PHPCXX_EXPORT placement_construct_t {
    explicit placement_construct_t() {}
};

PHPCXX_EXPORT extern const placement_construct_t placement_construct;

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
 * @}
 */

/**
 * @defgroup Construction of zvals
 * @{
 */
template<typename T, enable_if_t<is_null_pointer<T>::value>* = nullptr>
static inline void construct_zval(zval& z, T)   { ZVAL_NULL(&z); }

template<typename T, enable_if_t<is_bool<T>::value>* = nullptr>
static inline void construct_zval(zval& z, T v) { ZVAL_BOOL(&z, v); }

template<typename T, enable_if_t<is_integer<T>::value>* = nullptr>
static inline void construct_zval(zval& z, T v) { ZVAL_LONG(&z, v); }

template<typename T, enable_if_t<std::is_floating_point<T>::value>* = nullptr>
static inline void construct_zval(zval& z, T v) { ZVAL_DOUBLE(&z, v); }

template<typename T, enable_if_t<is_pchar<T>::value || is_string<T>::value>* = nullptr>
static inline void construct_zval(zval& z, T s)
{
    ZendString zs(s);
    ZVAL_STR(&z, zs.release());
}

template<typename T, enable_if_t<std::is_same<zval, remove_cv_t<T> >::value>* = nullptr>
static inline void construct_zval(zval& z, T s)
{
    ZVAL_COPY(&z, &s);
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
static void assign(zval* a, T b)
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
static void assign(zval* a, const T& b)
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

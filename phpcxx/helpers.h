#ifndef PHPCXX_HELPERS_H
#define PHPCXX_HELPERS_H

#include <Zend/zend.h>
#include <Zend/zend_string.h>
#include <cstring>
#include <type_traits>
#include "string.h"

namespace phpcxx {

// See http://loungecpp.wikidot.com/tips-and-tricks:indices
template<std::size_t... Is>
struct indices {};

template<std::size_t N, std::size_t... Is>
struct build_indices : build_indices<N-1, N-1, Is...> {};

template<std::size_t... Is>
struct build_indices<0, Is...> : indices<Is...> {};

template<typename ...T>
using IndicesFor = build_indices<sizeof...(T)>;

static inline void construct_zval(zval& z, std::nullptr_t) { ZVAL_NULL(&z); }
static inline void construct_zval(zval& z, bool v)         { ZVAL_BOOL(&z, v); }

template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
static inline void construct_zval(zval& z, T v) { ZVAL_LONG(&z, v); }

template<typename T, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
static inline void construct_zval(zval& z, T v) { ZVAL_DOUBLE(&z, v); }

static inline void construct_zval(zval& z, const char* s)
{
    zend_string* zs = zend_string_init(s, std::strlen(s), 0);
    ZVAL_STR(&z, zs);
}

static inline void construct_zval(zval& z, const phpcxx::string& s)
{
    zend_string* zs = zend_string_init(s.c_str(), s.size(), 0);
    ZVAL_STR(&z, zs);
}

}

#endif /* PHPCXX_HELPERS_H */

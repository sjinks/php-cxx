#ifndef PHPCXX_ARRAY_H
#define PHPCXX_ARRAY_H

#include "phpcxx.h"

#include <type_traits>
#include "helpers.h"
#include "string.h"
#include "map.h"
#include "vector.h"

#undef isset

namespace phpcxx {

class Value;
class ZendString;

/**
 * @brief Array
 */
class PHPCXX_EXPORT Array {
public:
    /**
     * @brief Constructs an empty array
     */
    Array();

    /**
     * @brief Constructs Array from @a z
     * @param z A pointer to `zval` to construct Array from
     * @details
     * - if @a z is an array, it is copied as is.
     * - if @a z is a reference to an array, @a z is dereferenced and copied
     * - otherwise, @a z is separated and converted to array
     */
    [[gnu::nonnull]] Array(zval* z);

    /**
     * @brief Constructs Array from `phpcxx::Value` @a v
     * @param v Value to construct Array from
     * @details
     * - if @a v is an array, it is copied as is.
     * - if @a v is a reference to an array, @a v is dereferenced and copied
     * - otherwise, @a v is separated and converted to array
     */
    Array(Value& v);

    /**
     * @brief Copy constructor
     * @param other Array being copied
     */
    Array(Array& other);

    /**
     * @brief Move constructor
     * @param other Array being moved
     */
    Array(Array&& other);

    /**
     * @brief Destructor
     */
    ~Array();

    /**
     * @brief Assignment operator
     * @param other Array being copied
     */
    Array& operator=(Array& other);

    /**
     * @brief Assignment operator
     * @tparam T Type of elements of the vector @a v
     * @param v Vector with values
     * @return Array
     */
    template<typename T>
    Array& operator=(const vector<T>& v);

    /**
     * @brief Assignment operator
     * @tparam K Type of the keys of the map @a v
     * @tparam V Type of the values of the map @a v
     * @param v Map with values
     * @return Array
     */
    template<typename K, typename V, enable_if_t<std::is_integral<K>::value>* = nullptr>
    Array& operator=(const map<K, V>& v);

    /**
     * @brief Assignment operator
     * @tparam K Type of the keys of the map @a v
     * @tparam V Type of the values of the map @a v
     * @param v Map with values
     * @return Array
     */
    template<typename K, typename V, enable_if_t<is_pchar<K>::value || is_string<K>::value>* = nullptr>
    Array& operator=(const map<K, V>& v);

    /**
     * @brief Adds an uninitialized value (`EG(uninitialized_zval)`) to the end of Array and returns a reference to that value
     * @return C++ reference to the added value
     * @throws std::runtime_error `zend_hash_next_index_insert` returned `FAILURE`
     */
    Value& operator[](std::nullptr_t);

    /**
     * @brief Returns a reference to the array item specified by @a idx.
     * @note If the item does not exist, it is silently created (`EG(uninitialized_zval)`)
     * @param idx Item index
     * @return C++ reference to the item
     * @throws std::runtime_error The item does not exist, and `zend_hash_index_add_new()` failed to create it
     */
    Value& operator[](zend_long idx);

    /**
     * @brief Returns a reference to the array item specified by @a key.
     * @note If the item does not exist, it is silently created (`EG(uninitialized_zval)`)
     * @param key Item key
     * @return C++ reference to the item
     * @throws std::runtime_error The item does not exists, and it was impossible to create it
     * @throws std::invalid_argument @a key is not scalar
     * @note This function invokes the proper overload of `operator[]` depending on the type of @a key
     */
    Value& operator[](const Value& key);

    /**
     * @brief Returns a reference to the array item specified by @a key.
     * @note If the item does not exist, it is silently created (`EG(uninitialized_zval)`)
     * @param key Item key
     * @return C++ reference to the item
     * @throws std::runtime_error The item does not exist, and attempt to create it failed
     * @note Numeric values are converted to `zend_long`
     */
    [[gnu::nonnull]] Value& operator[](zend_string* key);

    /**
     * @overload
     */
    Value& operator[](const char* key);

    /**
     * @overload
     */
    Value& operator[](const string& key);

    /**
     * @overload
     */
    Value& operator[](const ZendString& key);

    /**
     * @brief Returns a reference to the array item specified by @a idx.
     * @note If the item does not exist, it is silently created (`EG(uninitialized_zval)`)
     * @tparam T Type of the index (integral)
     * @param idx Item index
     * @return C++ reference to the item
     * @throws std::runtime_error The item does not exist, and `zend_hash_index_add_new()` failed to create it
     * @note This function is used in cases when `sizeof(idx) < sizeof(zend_long)`.
     */
    template<typename T, typename std::enable_if<(sizeof(T) < sizeof(zend_long)) && std::is_integral<T>::value>::type* = nullptr>
    Value& operator[](T idx)
    {
        return this->operator[](static_cast<zend_long>(idx));
    }

    /**
     * @brief Returns the size of the array
     * @return Number of elements in the array
     */
    std::size_t size() const;

    /**
     * @brief Checks whether the item with the index @a idx exists
     * @param idx Item index
     * @return Whether the item exists
     */
    bool isset(zend_long idx) const;

    /**
     * @brief Checks whether the item specified by @a key exists
     * @param key Item key or index
     * @return Whether the item exists
     * @note This function invokes the proper overload of `isset` depending on the type of @a key
     */
    bool isset(const Value& key) const;

    /**
     * @brief Checks whether the item with the key @a key exists
     * @param key Item key
     * @return Whether the item exists
     * @note Numeric values are converted to `zend_long`
     */
    [[gnu::nonnull]] bool isset(zend_string* key) const;

    /**
     * @overload
     */
    bool isset(const char* key) const;

    /**
     * @overload
     */
    bool isset(const string& key) const;

    /**
     * @overload
     */
    bool isset(const ZendString& key) const;

    /**
     * @brief Checks whether the item with the index @a idx exists
     * @tparam T Type of the index (integral)
     * @param idx Item index
     * @return Whether the item exists
     * @note This function is used in cases when `sizeof(idx) < sizeof(zend_long)`.
     */
    template<typename T, typename std::enable_if<(sizeof(T) < sizeof(zend_long)) && std::is_integral<T>::value>::type* = nullptr>
    bool isset(T idx)
    {
        return this->isset(static_cast<zend_long>(idx));
    }

    /**
     * @brief Removes the item specified by @a idx from the array
     * @param idx Item index
     */
    void unset(zend_long idx);

    /**
     * @brief Removes the item specified by @a key from the array
     * @param key Item index or key
     * @note This function invokes the proper overload of `unset` depending on the type of @a key
     */
    void unset(const Value& key);

    /**
     * @brief Removes the item specified by @a key from the array
     * @param key Item key
     * @note Numeric values are converted to `zend_long`
     */
    [[gnu::nonnull]] void unset(zend_string* key);

    /**
     * @overload
     */
    void unset(const char* key);

    /**
     * @overload
     */
    void unset(const string& key);

    /**
     * @overload
     */
    void unset(const ZendString& key);

    /**
     * @brief Removes the item specified by @a idx from the array
     * @param idx Item index
     * @note This function is used in cases when `sizeof(idx) < sizeof(zend_long)`.
     */
    template<typename T, typename std::enable_if<(sizeof(T) < sizeof(zend_long)) && std::is_integral<T>::value>::type* = nullptr>
    void unset(T idx)
    {
        this->unset(static_cast<zend_long>(idx));
    }

    /**
     * @brief Returns the internal representation of the array
     * @return `zval` which can be used with the Zend API
     * @warning The type of the returned `zval` must **not** be changed; doing so will result in a crash
     * @warning The `zval` is returned as is, its reference counter is not modified. Calling `zval_dtor()` or
     * `zval_ptr_dtor()` on the returned `zval` will result in a crash.
     */
    zval* pzval() { return &this->m_z; }
private:
    /**
     * @brief Internal representation of the array
     */
    zval m_z;

    /**
     * @brief Used internally for inplace construction with `placement new` operator
     * @internal
     */
    Array(placement_construct_t) {}

    friend class Value;
};

}

static_assert(sizeof(phpcxx::Array) == sizeof(zval), "sizeof(phpcxx::Array) must be equal to sizeof(zval)");

#include "array.tcc"

#endif /* PHPCXX_ARRAY_H */

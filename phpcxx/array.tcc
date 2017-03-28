#ifndef PHPCXX_ARRAY_TCC
#define PHPCXX_ARRAY_TCC

#ifndef PHPCXX_ARRAY_H
#error "Please do not include this file directly, use array.h instead"
#endif

namespace phpcxx {

template<typename T>
Array& Array::operator=(const vector<T>& v)
{
    zval* z = &this->m_z;
    ZVAL_DEREF(z);

    zval_ptr_dtor(z);
    _array_init(z, v.size() ZEND_FILE_LINE_CC);
    for (std::size_t i=0; i<v.size(); ++i) {
        zval x;
        construct_zval(x, v[i]);
        zend_hash_next_index_insert_new(Z_ARR_P(z), &x);
    }

    return *this;
}

template<typename K, typename V, enable_if_t<std::is_integral<K>::value>*>
Array& Array::operator=(const map<K, V>& v)
{
    zval* z = &this->m_z;
    ZVAL_DEREF(z);

    zval_ptr_dtor(z);
    _array_init(z, v.size() ZEND_FILE_LINE_CC);
    for (auto const& it : v) {
        zval x;
        construct_zval(x, it.second);
        zend_hash_index_add(Z_ARR_P(z), static_cast<zend_ulong>(it.first), &x);
    }

    return *this;
}

template<typename K, typename V, enable_if_t<is_pchar<K>::value || is_string<K>::value>*>
Array& Array::operator=(const map<K, V>& v)
{
    zval* z = &this->m_z;
    ZVAL_DEREF(z);

    zval_ptr_dtor(z);
    _array_init(z, v.size() ZEND_FILE_LINE_CC);
    for (auto const& it : v) {
        zval x;
        construct_zval(x, it.second);
        ZendString key(it.first);
        zend_hash_add(Z_ARR_P(z), key.get(), &x);
    }

    return *this;
}

}

#endif /* PHPCXX_ARRAY_TCC */

#ifndef PHPCXX_OBJECT_H
#define PHPCXX_OBJECT_H

#include "phpcxx.h"

#include "helpers.h"
#include "map.h"
#include "string.h"

namespace phpcxx {

class ObjectDimension;
class Value;
class ZendString;

class PHPCXX_EXPORT Object {
public:
    Object();
    [[gnu::nonnull]] Object(zval* z);
    Object(Value& v);
    Object(Object& other);
    Object(Object&& other);

    ~Object();

    Object& operator=(const Object& other);

    ObjectDimension operator[](const char* key);
    ObjectDimension operator[](const string& key);
    ObjectDimension operator[](const ZendString& key);

    [[gnu::nonnull]] ObjectDimension operator[](zend_string* key);
    ObjectDimension operator[](zend_ulong key);

    bool offsetExists(zend_long idx) const;
    bool offsetExists(const Value& key) const;
    [[gnu::nonnull]] bool offsetExists(zend_string* key) const;

    Value offsetGet(zend_long idx);
    Value offsetGet(const Value& key);
    [[gnu::nonnull]] Value offsetGet(zend_string* key);

    void offsetSet(zend_long idx, const Value& v);
    void offsetSet(const Value& key, const Value& v);
    void offsetSet(std::nullptr_t, const Value& v);
    [[gnu::nonnull]] void offsetSet(zend_string* key, const Value& v);

    void offsetUnset(zend_long idx);
    void offsetUnset(const Value& key);
    [[gnu::nonnull]] void offsetUnset(zend_string* key);

    bool isSet();
    Value& get();
    void set();
    void unset();

    zend_long count();

    [[gnu::returns_nonnull]] zval* pzval() const { return &this->m_z; }

private:
    mutable zval m_z;

    Object(placement_construct_t) {}

    friend class Value;
};

}



#endif /* PHPCXX_OBJECT_H */

#ifndef PHPCXX_ARRAY_H_
#define PHPCXX_ARRAY_H_

#include "phpcxx.h"

#include <Zend/zend.h>
#include <Zend/zend_long.h>
#include <Zend/zend_string.h>
#include "string.h"
#include "zendstring.h"

namespace phpcxx {

class Value;

class PHPCXX_EXPORT Array {
public:
    Array();
    [[gnu::nonnull]] Array(zval* z);
    Array(Value& v);
    Array(Array& other);
    Array(Array&& other);

    ~Array();

    Array& operator=(const Array& other);

    Value& operator[](nullptr_t);
    Value& operator[](zend_long idx);
    Value& operator[](const Value& key);
    [[gnu::nonnull]] Value& operator[](zend_string* key);

    Value& operator[](const string& key)     { return this->operator[](ZendString(key)); }
    Value& operator[](const char* key)       { return this->operator[](ZendString(key)); }
    Value& operator[](const ZendString& key) { return this->operator[](key.get());       }

    std::size_t size() const;

    bool contains(zend_long idx) const;
    bool contains(const Value& key) const;
    [[gnu::nonnull]] bool contains(zend_string* key) const;

    bool contains(const string& key) const     { return this->contains(ZendString(key)); }
    bool contains(const char* key) const       { return this->contains(ZendString(key)); }
    bool contains(const ZendString& key) const { return this->contains(key.get());       }

    void unset(zend_long idx);
    void unset(const Value& key);
    [[gnu::nonnull]] void unset(zend_string* key);

    void unset(const string& key)     { this->unset(ZendString(key)); }
    void unset(const char* key)       { this->unset(ZendString(key)); }
    void unset(const ZendString& key) { this->unset(key.get()); }
private:
    mutable zval m_z;

    Array(nullptr_t); // for placement new

    friend class Value;
};

}

#endif /* PHPCXX_ARRAY_H_ */

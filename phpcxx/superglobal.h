#ifndef PHPCXX_SUPERGLOBAL_H
#define PHPCXX_SUPERGLOBAL_H

#include "phpcxx.h"

#include "value.h"

#undef isset

namespace phpcxx {

class PHPCXX_EXPORT SuperGlobal {
public:
    explicit SuperGlobal(int idx);
    [[gnu::nonnull]] SuperGlobal(const char* name);

    zval* pzval() const;
    Value& get() const;
    std::size_t size() const;
    Type type() const;

    Value& operator[](std::nullptr_t);
    Value& operator[](zend_long idx);
    Value& operator[](const Value& key);
    [[gnu::nonnull]] Value& operator[](zend_string* key);

    bool isset(zend_long idx) const;
    bool isset(const Value& key) const;
    [[gnu::nonnull]] bool isset(zend_string* key) const;

    void unset(zend_long idx);
    void unset(const Value& key);
    [[gnu::nonnull]] void unset(zend_string* key);

    Value& operator[](const char* key);
    Value& operator[](const string& key);
    Value& operator[](const ZendString& key);

    bool isset(const char* key) const;
    bool isset(const string& key) const;
    bool isset(const ZendString& key) const;

    void unset(const char* key);
    void unset(const string& key);
    void unset(const ZendString& key);

private:
    const char* m_name;
    mutable zval* m_z;
    int m_idx;
};

extern PHPCXX_EXPORT SuperGlobal orig_POST;
extern PHPCXX_EXPORT SuperGlobal orig_GET;
extern PHPCXX_EXPORT SuperGlobal orig_COOKIE;
extern PHPCXX_EXPORT SuperGlobal orig_SERVER;
extern PHPCXX_EXPORT SuperGlobal orig_ENV;
extern PHPCXX_EXPORT SuperGlobal orig_FILES;
extern PHPCXX_EXPORT SuperGlobal _POST;
extern PHPCXX_EXPORT SuperGlobal _GET;
extern PHPCXX_EXPORT SuperGlobal _COOKIE;
extern PHPCXX_EXPORT SuperGlobal _SERVER;
extern PHPCXX_EXPORT SuperGlobal _ENV;
extern PHPCXX_EXPORT SuperGlobal _FILES;
extern PHPCXX_EXPORT SuperGlobal _REQUEST;
extern PHPCXX_EXPORT SuperGlobal _SESSION;
extern PHPCXX_EXPORT SuperGlobal GLOBALS;

}

#endif /* PHPCXX_SUPERGLOBAL_H */

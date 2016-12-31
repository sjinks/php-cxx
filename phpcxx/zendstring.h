#ifndef PHPCXX_ZENDSTRING_H
#define PHPCXX_ZENDSTRING_H

#include "phpcxx.h"

#include <Zend/zend_string.h>
#include <cstring>
#include <string>
#include "string.h"

namespace phpcxx {

class PHPCXX_EXPORT ZendString {
public:
    ZendString(const char* s)
        : m_s(zend_string_init(s, std::strlen(s), 0))
    {
    }

    ZendString(const std::string& s)
        : m_s(zend_string_init(s.c_str(), s.size(), 0))
    {
    }

    ZendString(const string& s)
        : m_s(zend_string_init(s.c_str(), s.size(), 0))
    {
    }

    ZendString(const ZendString& other)
        : m_s(zend_string_copy(other.m_s))
    {
    }

    ZendString(ZendString&& other)
        : m_s(std::move(other.m_s))
    {
        other.m_s = nullptr;
    }

    ~ZendString()
    {
        this->maybeReleaseString();
    }

    ZendString& operator=(const ZendString& other)
    {
        if (this != &other) {
            this->maybeReleaseString();
            this->m_s = zend_string_copy(other.m_s);
        }

        return *this;
    }

    ZendString& operator=(ZendString&& other)
    {
        if (this != &other) {
            this->maybeReleaseString();
            this->m_s = std::move(other.m_s);
            other.m_s = nullptr;
        }

        return *this;
    }

    ZendString& operator=(const char* s)
    {
        this->maybeReleaseString();
        this->m_s = zend_string_init(s, std::strlen(s), 0);
        return *this;
    }

    ZendString& operator=(const std::string& s)
    {
        this->maybeReleaseString();
        this->m_s = zend_string_init(s.c_str(), s.size(), 0);
        return *this;
    }

    ZendString& operator=(const string& s)
    {
        this->maybeReleaseString();
        this->m_s = zend_string_init(s.c_str(), s.size(), 0);
        return *this;
    }

    zend_string* get()
    {
        return this->m_s;
    }

    zend_string* get() const
    {
        return this->m_s;
    }

private:
    mutable zend_string* m_s;

    void maybeReleaseString()
    {
        if (this->m_s) {
            zend_string_release(this->m_s);
        }
    }
};

}

#endif /* PHPCXX_ZENDSTRING_H */
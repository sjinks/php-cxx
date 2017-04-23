#ifndef PHPCXX_ZENDSTRING_H
#define PHPCXX_ZENDSTRING_H

#include "phpcxx.h"

#include <cstring>
#include <string>
#include "string.h"

extern "C" {
#include <Zend/zend_string.h>
}

namespace phpcxx {

/**
 * @brief Wrapper for `zend_string`
 */
class PHPCXX_EXPORT ZendString {
public:
    /**
     * @brief Creates a new `zend_string` from @a s
     * @param s Null-terminated C string
     */
    ZendString(const char* s)
        : m_s(zend_string_init(s, s ? std::strlen(s) : 0, 0))
    {
#ifdef HAVE_BUILTIN_CONSTANT_P
        if (__builtin_constant_p(s)) {
            ZSTR_H(this->m_s) = zend_inline_hash_func(s, std::strlen(s));
        }
#endif
    }

    /**
     * @brief Creates a new `zend_string` from @a s
     * @param s std::string
     */
    ZendString(const std::string& s)
        : m_s(zend_string_init(s.c_str(), s.size(), 0))
    {
    }

    /**
     * @brief Creates a new `zend_string` from @a s
     * @param s phpcxx::string
     */
    ZendString(const string& s)
        : m_s(zend_string_init(s.c_str(), s.size(), 0))
    {
    }

    /**
     * @brief Creates a new `zend_string` from @a s
     * @param s zend_string
     */
    ZendString(zend_string* s)
        : m_s(s ? zend_string_copy(s) : s)
    {
    }

    /**
     * @brief Copy constructor
     * @param other String being copied
     */
    ZendString(const ZendString& other)
        : m_s(zend_string_copy(other.m_s))
    {
    }

    /**
     * @brief Move constructor
     * @param other String being moved
     */
    ZendString(ZendString&& other) noexcept
        : m_s(std::move(other.m_s))
    {
        other.m_s = nullptr;
    }

    /**
     * @brief Destructor
     */
    ~ZendString()
    {
        this->maybeReleaseString();
    }

    /**
     * @brief Assignment operator
     * @param other String being assigned
     * @return ZendString
     */
    ZendString& operator=(const ZendString& other)
    {
        if (this != &other) {
            this->maybeReleaseString();
            this->m_s = zend_string_copy(other.m_s);
        }

        return *this;
    }

    /**
     * @brief Move-assignment operator
     * @param other String being assigned
     * @return ZendString
     */
    ZendString& operator=(ZendString&& other)
    {
        if (this != &other) {
            this->maybeReleaseString();
            this->m_s = std::move(other.m_s);
            other.m_s = nullptr;
        }

        return *this;
    }

    /**
     * @brief Assignment operator
     * @param s Null-terminated C string
     */
    ZendString& operator=(const char* s)
    {
        this->maybeReleaseString();
        this->m_s = zend_string_init(s, s ? std::strlen(s) : 0, 0);
        return *this;
    }

    /**
     * @brief Assignment operator
     * @param s std::string
     */
    ZendString& operator=(const std::string& s)
    {
        this->maybeReleaseString();
        this->m_s = zend_string_init(s.c_str(), s.size(), 0);
        return *this;
    }

    /**
     * @brief Assignment operator
     * @param s phpcxx::string
     */
    ZendString& operator=(const string& s)
    {
        this->maybeReleaseString();
        this->m_s = zend_string_init(s.c_str(), s.size(), 0);
        return *this;
    }

    /**
     * @brief Assignment operator
     * @param s phpcxx::string
     */
    ZendString& operator=(zend_string* s)
    {
        this->maybeReleaseString();
        this->m_s = s ? zend_string_copy(s) : s;
        return *this;
    }

    /** @name Interned Strings */
    /**@{*/

    /**
     * @brief Tries to make the underlying Zend string interned
     */
    void makeInterned()
    {
        if (this->m_s) {
            this->m_s = zend_new_interned_string(this->m_s);
        }
    }

    /**
     * @brief Checks whether the string is interned
     * @return Whether the string is interned
     */
    bool isInterned() const
    {
        return this->m_s ? ZSTR_IS_INTERNED(this->m_s) : false;
    }
    /**@}*/

    /**
     * @brief Returns the underlying Zend string
     * @return Zend string
     */
    zend_string* get()
    {
        return this->m_s;
    }

    /**
     * @brief Returns the underlying Zend string
     * @return Zend string
     */
    zend_string* get() const
    {
        return this->m_s;
    }

    /**
     * @brief Returns the underlying Zend string and transfers its ownership to the caller
     * @return Zend string
     */
    zend_string* release()
    {
        zend_string* s = this->m_s;
        this->m_s      = nullptr;
        return s;
    }

    /**
     * @brief Compares @a rhs for equality
     * @param rhs Other ZendString
     * @return Whether two ZendStrings are equal
     */
    bool operator==(const ZendString& rhs) const
    {
        return zend_string_equals(this->m_s, rhs.m_s);
    }

    /** @name Reference counting */
    /**@{*/

    /**
     * @brief Returns the number of references to the string
     * @return Number of references
     * @retval 0 if `ZendString` does not own any string
     */
    uint32_t refCount() const
    {
        return this->m_s ? zend_string_refcount(this->m_s) : 0;
    }

    /**
     * @brief Increments the reference count
     * @return Number of references
     * @retval 0 if `ZendString` does not own any string
     */
    uint32_t addRef()
    {
        return this->m_s ? zend_string_addref(this->m_s) : 0;
    }

    /**
     * @brief Decrements the reference count
     * @return Number of references
     * @retval 0 if `ZendString` does not own any string
     */
    uint32_t delRef()
    {
        return this->m_s ? zend_string_delref(this->m_s) : 0;
    }
    /**@}*/

private:
    /**
     * @brief Zend String
     */
    mutable zend_string* m_s;

    /**
     * @brief Releases `zend_string` if it is not `nullptr`
     */
    void maybeReleaseString()
    {
        if (this->m_s) {
            zend_string_release(this->m_s);
        }
    }
};

}

#endif /* PHPCXX_ZENDSTRING_H */

#ifndef PHPCXX_VALUE_H_
#define PHPCXX_VALUE_H_

#include "phpcxx.h"

#include <Zend/zend.h>
#include <Zend/zend_API.h>
#include <Zend/zend_operators.h>
#include <array>
#include <string>
#include "array.h"
#include "helpers.h"
#include "operators.h"
#include "phpexception.h"
#include "string.h"
#include "types.h"
#include "zendstring.h"

namespace phpcxx {

class PHPCXX_EXPORT Value {
public:
    Value()
    {
        FTRACE();
        ZVAL_UNDEF(&this->m_z);
    }

    Value(Type t)
    {
        FTRACE();
        switch (t) {
            case Type::Null:      ZVAL_NULL(&this->m_z);          return;
            case Type::False:     ZVAL_FALSE(&this->m_z);         return;
            case Type::True:      ZVAL_TRUE(&this->m_z);          return;
            case Type::Integer:   ZVAL_LONG(&this->m_z, 0);       return;
            case Type::Double:    ZVAL_DOUBLE(&this->m_z, 0);     return;
            case Type::String:    ZVAL_EMPTY_STRING(&this->m_z);  return;
            case Type::Array:     array_init(&this->m_z);         return;
            case Type::Object:    object_init(&this->m_z);        return;
            case Type::Reference: ZVAL_NEW_EMPTY_REF(&this->m_z); return;
            case Type::Bool:      ZVAL_FALSE(&this->m_z);         return;
            default: // Resource, Constant, ConstantAST, Callable, Indirect, Pointer
            case Type::Undefined: ZVAL_UNDEF(&this->m_z); return;
        }
    }

    Value(zval* z, CopyPolicy policy = CopyPolicy::Assign)
    {
        FTRACE();
        switch (policy) {
            case CopyPolicy::Assign:
                if (Z_ISREF_P(z)) {
                    z = Z_REFVAL_P(z);
                }

                ZVAL_COPY(&this->m_z, z);
                break;

            case CopyPolicy::Copy:
                ZVAL_COPY(&this->m_z, z);
                break;

            case CopyPolicy::Reference: {
                ZVAL_MAKE_REF(z);
                zend_reference* ref = Z_REF_P(z);
                ++GC_REFCOUNT(ref);
                ZVAL_REF(&this->m_z, ref);
                break;
            }

            case CopyPolicy::Wrap:
                ZVAL_COPY_VALUE(&this->m_z, z);
                break;

            case CopyPolicy::Duplicate:
                ZVAL_DUP(&this->m_z, z);
                break;
        }
    }

    template<typename T>
    Value(const T& v)
    {
        FTRACE();
        construct_zval(this->m_z, v);
    }

    Value(Value&& other) noexcept
        : m_z(other.m_z)
    {
        FTRACE();
        ZVAL_UNDEF(&other.m_z);
    }

    Value(const Value& other) noexcept
    {
        FTRACE();
        const zval* z = &other.m_z;
        if (Z_ISREF_P(z)) {
            z = Z_REFVAL_P(z);
        }

        ZVAL_COPY(&this->m_z, z);
    }

    /// TODO: policy = CopyPolicy::Wrap is asking for trouble
    Value(Value& other, CopyPolicy policy)
        : Value(&other.m_z, policy)
    {
    }

    ~Value()
    {
        FTRACE();
        i_zval_ptr_dtor(&this->m_z ZEND_FILE_LINE_CC);
#ifdef PHPCXX_DEBUG
        ZVAL_UNDEF(&this->m_z);
#endif
    }

    void assignTo(zval* a)
    {
        phpcxx::assign(a, &this->m_z);
    }

    /*
     * zend_object_set_t requires zval*, not const zval*
     */
    Value& operator=(zval* b)
    {
        FTRACE();
        phpcxx::assign(&this->m_z, b);
        return *this;
    }

    Value& operator=(Value& other)
    {
        FTRACE();
        return operator=(&other.m_z);
    }

    Value& operator=(Value&& other)
    {
        FTRACE();
        if (this == &other) {
            return *this;
        }

        if (!this->isRefcounted()) {
            std::swap(this->m_z, other.m_z);
            return *this;
        }

        return operator=(&other.m_z);
    }

    template<typename T>
    Value& operator=(T v)
    {
        FTRACE();
        if (!this->isRefcounted()) {
            construct_zval(this->m_z, v);
            return *this;
        }

        phpcxx::assign(&this->m_z, v);
        return *this;
    }

    Value reference()
    {
        return Value(*this, CopyPolicy::Reference);
    }

    Value& dereference()
    {
        zval* a = &this->m_z;
        if (Z_ISREF_P(a)) {
            a = Z_REFVAL_P(a);
        }
        else {
            a = &EG(error_zval);
        }

        return *(new(a) Value(nullptr));
    }

    static Value createReference(Value& to)
    {
        return to.reference();
    }

    Type type() const
    {
        return static_cast<Type>(Z_TYPE(this->m_z));
    }

    void becomeReference()
    {
        ZVAL_MAKE_REF(&this->m_z);
    }

    void unreference()
    {
        if (this->isReference()) {
            if (this->refCount() == 1) {
                ZVAL_UNREF(&this->m_z);
            }
            else {
                Z_DELREF(this->m_z);
                ZVAL_DUP(&this->m_z, Z_REFVAL(this->m_z));
            }
        }
    }

    bool isRefcounted() const
    {
        return Z_REFCOUNTED(this->m_z);
    }

    bool isReference() const
    {
        return Z_ISREF(this->m_z);
    }

    bool isCollectable() const
    {
        return Z_COLLECTABLE(this->m_z);
    }

    bool isCopyable() const
    {
        return Z_COPYABLE(this->m_z);
    }

    bool isImmutable() const
    {
        return Z_IMMUTABLE(this->m_z);
    }

    bool isIndirect() const
    {
        return Z_TYPE(this->m_z) == IS_INDIRECT;
    }

    uint32_t refCount() const
    {
        return this->isRefcounted() ? GC_REFCOUNT(Z_COUNTED(this->m_z)) : 0;
    }

    string toString() const
    {
        if (Z_TYPE(this->m_z) == IS_UNDEF) {
            return "[unset]";
        }

        zend_string* s = zval_get_string(const_cast<zval*>(&this->m_z));
        string ret(ZSTR_VAL(s), ZSTR_LEN(s));
        zend_string_release(s);
        return ret;
    }

    Value& operator+=(const Value& rhs);
    Value& operator-=(const Value& rhs);
    Value& operator*=(const Value& rhs);
    Value& operator/=(const Value& rhs);
    Value& operator%=(const Value& rhs);
    Value& operator^=(const Value& rhs);
    Value& operator&=(const Value& rhs);
    Value& operator|=(const Value& rhs);
    Value& operator<<=(const Value& rhs);
    Value& operator>>=(const Value& rhs);

    friend Value   operator+(const Value& lhs, const Value& rhs) { Value t(lhs); t += rhs; return t;  }
    friend Value&& operator+(Value&& lhs, const Value& rhs)      { lhs += rhs; return std::move(lhs); }
    friend Value&& operator+(const Value& lhs, Value&& rhs)      { rhs += lhs; return std::move(rhs); }
    friend Value&& operator+(Value&& lhs, Value&& rhs)           { lhs += rhs; return std::move(lhs); }

    friend Value   operator-(const Value& lhs, const Value& rhs) { Value t(lhs); t -= rhs; return t;  }
    friend Value&& operator-(Value&& lhs, const Value& rhs)      { lhs -= rhs; return std::move(lhs); }
    friend Value&& operator-(Value&& lhs, Value&& rhs)           { lhs -= rhs; return std::move(lhs); }

    friend Value   operator*(const Value& lhs, const Value& rhs) { Value t(lhs); t *= rhs; return t;  }
    friend Value&& operator*(Value&& lhs, const Value& rhs)      { lhs *= rhs; return std::move(lhs); }
    friend Value&& operator*(const Value& lhs, Value&& rhs)      { rhs *= lhs; return std::move(rhs); }
    friend Value&& operator*(Value&& lhs, Value&& rhs)           { lhs *= rhs; return std::move(lhs); }

    friend Value   operator/(const Value& lhs, const Value& rhs) { Value t(lhs); t /= rhs; return t;  }
    friend Value&& operator/(Value&& lhs, const Value& rhs)      { lhs /= rhs; return std::move(lhs); }
    friend Value&& operator/(Value&& lhs, Value&& rhs)           { lhs /= rhs; return std::move(lhs); }

    friend Value   operator%(const Value& lhs, const Value& rhs) { Value t(lhs); t %= rhs; return t;  }
    friend Value&& operator%(Value&& lhs, const Value& rhs)      { lhs %= rhs; return std::move(lhs); }
    friend Value&& operator%(Value&& lhs, Value&& rhs)           { lhs %= rhs; return std::move(lhs); }

    friend Value   operator^(const Value& lhs, const Value& rhs) { Value t(lhs); t ^= rhs; return t;  }
    friend Value&& operator^(Value&& lhs, const Value& rhs)      { lhs ^= rhs; return std::move(lhs); }
    friend Value&& operator^(const Value& lhs, Value&& rhs)      { rhs ^= lhs; return std::move(rhs); }
    friend Value&& operator^(Value&& lhs, Value&& rhs)           { lhs ^= rhs; return std::move(lhs); }

    friend Value   operator&(const Value& lhs, const Value& rhs) { Value t(lhs); t &= rhs; return t;  }
    friend Value&& operator&(Value&& lhs, const Value& rhs)      { lhs &= rhs; return std::move(lhs); }
    friend Value&& operator&(const Value& lhs, Value&& rhs)      { rhs &= lhs; return std::move(rhs); }
    friend Value&& operator&(Value&& lhs, Value&& rhs)           { lhs &= rhs; return std::move(lhs); }

    friend Value   operator|(const Value& lhs, const Value& rhs) { Value t(lhs); t |= rhs; return t;  }
    friend Value&& operator|(Value&& lhs, const Value& rhs)      { lhs |= rhs; return std::move(lhs); }
    friend Value&& operator|(const Value& lhs, Value&& rhs)      { rhs |= lhs; return std::move(rhs); }
    friend Value&& operator|(Value&& lhs, Value&& rhs)           { lhs |= rhs; return std::move(lhs); }

    friend Value   operator<<(const Value& lhs, const Value& rhs) { Value t(lhs); t <<= rhs; return t;  }
    friend Value&& operator<<(Value&& lhs, const Value& rhs)      { lhs <<= rhs; return std::move(lhs); }
    friend Value&& operator<<(Value&& lhs, Value&& rhs)           { lhs <<= rhs; return std::move(lhs); }

    friend Value   operator>>(const Value& lhs, const Value& rhs) { Value t(lhs); t >>= rhs; return t;  }
    friend Value&& operator>>(Value&& lhs, const Value& rhs)      { lhs >>= rhs; return std::move(lhs); }
    friend Value&& operator>>(Value&& lhs, Value&& rhs)           { lhs >>= rhs; return std::move(lhs); }

    friend Value pow(Value lhs, const Value& rhs);
    friend Value concat(Value lhs, const Value& rhs);
    friend int compare(const Value& lhs, const Value& rhs);

    bool isIdenticalTo(const Value& rhs);
    bool isNotIdenticalTo(const Value& rhs);

    // instanceof

    Value& operator++();
    Value& operator--();

    Value operator++(int)
    {
        Value tmp(*this);
        this->operator++();
        return tmp;
    }

    Value operator--(int)
    {
        Value tmp(*this);
        this->operator--();
        return tmp;
    }

    zend_long asLong() const
    {
        return zval_get_long(const_cast<zval*>(&this->m_z));
    }

    double asDouble() const
    {
        return zval_get_double(const_cast<zval*>(&this->m_z));
    }

    string asString() const
    {
        return this->toString();
    }

    Value& operator[](const std::string& key)
    {
        return this->operator[](ZendString(key));
    }

    Value& operator[](const string& key)
    {
        return this->operator[](ZendString(key));
    }

    Value& operator[](const char* key)
    {
        return this->operator[](ZendString(key));
    }

    Value& operator[](int key)
    {
        return this->operator[](static_cast<zend_long>(key));
    }

    Value& operator[](const ZendString& key);
    Value& operator[](zend_long key);
    Value& operator[](std::nullptr_t);

    template<typename ...Args>
    Value operator()(Args&&... args)
    {
        return this->_call(nullptr, IndicesFor<Args...>{}, std::forward<Args>(args)...);
    }

    std::string debugZval() const;
    static string typeToString(Type type);

    /**
     * @internal
     */
    zval* getZVal() { return &this->m_z; }

private:
    mutable zval m_z;

    struct placement_construction_t {};

    Value(const placement_construction_t&) {}

    friend phpcxx::Value operator~(const phpcxx::Value& op);
    friend phpcxx::Value operator!(const phpcxx::Value& op);
    friend bool operator==(const phpcxx::Value& lhs, const phpcxx::Value& rhs);
    friend bool operator!=(const phpcxx::Value& lhs, const phpcxx::Value& rhs);
    friend bool operator<(const phpcxx::Value& lhs, const phpcxx::Value& rhs);
    friend bool operator<=(const phpcxx::Value& lhs, const phpcxx::Value& rhs);
    friend bool operator>(const phpcxx::Value& lhs, const phpcxx::Value& rhs);
    friend bool operator>=(const phpcxx::Value& lhs, const phpcxx::Value& rhs);

    friend class Array;

    template<typename ...Args, std::size_t ...Is>
    Value _call(zval* object, indices<Is...>, Args&&... args)
    {
        std::array<zval, sizeof...(args)> zparams;
        zparams = { { *Value::paramHelper(std::move(args), zparams[Is])... } };

        zval retval;
        if (call_user_function_ex(CG(function_table), object, &this->m_z, &retval, zparams.size(), zparams.data(), 1, nullptr) == SUCCESS) {
            if (UNEXPECTED(EG(exception))) {
                i_zval_ptr_dtor(&retval ZEND_FILE_LINE_CC);
                throw PhpException();
            }

            return Value(&retval, CopyPolicy::Wrap);
        }

        return Value();
    }

    static zval* paramHelper(Value&& v, zval&) { return &v.m_z; }
    static zval* paramHelper(Array&& v, zval&) { return &v.m_z; }

    template<typename T>
    static zval* paramHelper(const T& v, zval& z) { construct_zval(z, v); Z_TRY_DELREF(z); return &z; }

    friend void construct_zval(zval& z, const Value& v) { ZVAL_COPY(&z, &v.m_z); }
};

extern Value ErrorValue;

}

#endif /* PHPCXX */

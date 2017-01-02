#ifndef PHPCXX_VALUE_H_
#define PHPCXX_VALUE_H_

#include "phpcxx.h"

#include <Zend/zend.h>
#include <Zend/zend_API.h>
#include <Zend/zend_operators.h>
#include <array>
#include <string>
#include "phpexception.h"
#include "string.h"
#include "types.h"
#include "helpers.h"
#include "zendstring.h"

namespace phpcxx {

class PHPCXX_EXPORT Value {
public:
    Value()
    {
        ZVAL_UNDEF(&this->m_z);
    }

    Value(std::nullptr_t)
    {
        construct_zval(this->m_z, nullptr);
    }

    Value(zval* z, CopyPolicy policy = CopyPolicy::Assign)
    {
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
        construct_zval(this->m_z, v);
    }

    Value(Value&& other) noexcept
        : m_z(other.m_z)
    {
        ZVAL_UNDEF(&other.m_z);
    }

    Value(const Value& other) noexcept
    {
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
        i_zval_ptr_dtor(&this->m_z ZEND_FILE_LINE_CC);
    }

    static void assign(zval* a, zval* b, AssignPolicy policy = AssignPolicy::AssignVariable)
    {
        if (Z_ISREF_P(b)) {
            b = Z_REFVAL_P(b);
        }

        if (Z_ISREF_P(a)) {
            a = Z_REFVAL_P(a);
        }

        if (Z_IMMUTABLE_P(a)) {
            zend_error(E_ERROR, "Cannot assign to an immutable variable");
            ZEND_ASSUME(0); // unreachable
        }

        if (AssignPolicy::AssignVariable == policy) {
            if (Z_REFCOUNTED_P(a)) {
                if (Z_TYPE_P(a) == IS_OBJECT && Z_OBJ_HANDLER_P(a, set)) {
                    Z_OBJ_HANDLER_P(a, set)(a, b);
                    return;
                }

                if (a == b) {
                    return;
                }

                if (Z_REFCOUNT_P(a) == 1) {
                    zval_dtor(a);
                    ZVAL_COPY(a, b);
                    return;
                }

                zval_ptr_dtor(a);
                zval_copy_ctor_func(a);
            }

            ZVAL_COPY(a, b);
        }
        else {
            if (Z_REFCOUNTED_P(a)) {
                if (Z_TYPE_P(a) == IS_OBJECT && Z_OBJ_HANDLER_P(a, set)) {
                    Z_OBJ_HANDLER_P(a, set)(a, b);
                    zval_ptr_dtor(b);
                    return;
                }

                if (a == b) {
                    return;
                }

                if (Z_REFCOUNT_P(a) == 1) {
                    zval_dtor(a);
                    ZVAL_COPY_VALUE(a, b);
                    return;
                }

                zval_ptr_dtor(a);
            }

            ZVAL_COPY_VALUE(a, b);
        }
    }

    Value& assign(zval* b, AssignPolicy policy = AssignPolicy::AssignVariable)
    {
        zval* a = &this->m_z;
        Value::assign(a, b, policy);
        return *this;
    }

    void assignTo(zval* a)
    {
        Value::assign(a, &this->m_z);
    }

    /*
     * zend_object_set_t requires zval*, not const zval*
     */
    Value& operator=(zval* b)
    {
        return assign(b, AssignPolicy::AssignVariable);
    }

    Value& operator=(Value& other)
    {
        return operator=(&other.m_z);
    }

    Value& operator=(Value&& other)
    {
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
        zval z;
        if (!this->isRefcounted()) {
            construct_zval(this->m_z, v);
            return *this;
        }

        construct_zval(z, v);
        return this->assign(&z, AssignPolicy::AssignTemporary);
    }

    static Value createReference(Value& to)
    {
        Value res;
        zval* b = &to.m_z;
        zval* a = &res.m_z;

        ZVAL_MAKE_REF(b);
        zend_reference* ref = Z_REF_P(b);
        ++GC_REFCOUNT(ref);
        ZVAL_REF(a, ref);
        return res;
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
        zend_string* s = zval_get_string(const_cast<zval*>(&this->m_z));
        string ret(ZSTR_VAL(s), ZSTR_LEN(s));
        zend_string_release(s);
        return ret;
    }

    Value& operator+=(const Value& rhs)  { return binaryOperator(fast_add_function, rhs); }
    Value& operator-=(const Value& rhs)  { return binaryOperator(sub_function, rhs); }
    Value& operator*=(const Value& rhs)  { return binaryOperator(mul_function, rhs); }
    Value& operator/=(const Value& rhs)  { return binaryOperator(fast_div_function, rhs); }
    Value& operator%=(const Value& rhs)  { return binaryOperator(mod_function, rhs); }
    Value& operator^=(const Value& rhs)  { return binaryOperator(bitwise_xor_function, rhs); }
    Value& operator&=(const Value& rhs)  { return binaryOperator(bitwise_and_function, rhs); }
    Value& operator|=(const Value& rhs)  { return binaryOperator(bitwise_or_function, rhs); }
    Value& operator<<=(const Value& rhs) { return binaryOperator(shift_left_function, rhs); }
    Value& operator>>=(const Value& rhs) { return binaryOperator(shift_right_function, rhs); }

    friend Value operator+(Value lhs, const Value& rhs) { lhs += rhs; return lhs; }
    friend Value operator-(Value lhs, const Value& rhs) { lhs -= rhs; return lhs; }
    friend Value operator*(Value lhs, const Value& rhs) { lhs *= rhs; return lhs; }
    friend Value operator/(Value lhs, const Value& rhs) { lhs /= rhs; return lhs; }
    friend Value operator%(Value lhs, const Value& rhs) { lhs %= rhs; return lhs; }
    friend Value operator^(Value lhs, const Value& rhs) { lhs ^= rhs; return lhs; }
    friend Value operator&(Value lhs, const Value& rhs) { lhs &= rhs; return lhs; }
    friend Value operator|(Value lhs, const Value& rhs) { lhs |= rhs; return lhs; }
    friend Value operator<<(Value lhs, const Value& rhs) { lhs <<= rhs; return lhs; }
    friend Value operator>>(Value lhs, const Value& rhs) { lhs >>= rhs; return lhs; }

    friend bool operator==(const Value& lhs, const Value& rhs) { return compareOperator(is_equal_function, lhs, rhs); }
    friend bool operator!=(const Value& lhs, const Value& rhs) { return compareOperator(is_not_equal_function, lhs, rhs); }
    friend bool operator<(const Value& lhs, const Value& rhs)  { return compareOperator(is_smaller_function, lhs, rhs); }
    friend bool operator<=(const Value& lhs, const Value& rhs) { return compareOperator(is_smaller_or_equal_function, lhs, rhs); }
    friend bool operator>(const Value& lhs, const Value& rhs)  { return rhs < lhs; }
    friend bool operator>=(const Value& lhs, const Value& rhs) { return rhs <= lhs; }
    bool isIdenticalTo(const Value& rhs)    { return compareOperator(is_identical_function, *this, rhs); }
    bool isNotIdenticalTo(const Value& rhs) { return compareOperator(is_not_identical_function, *this, rhs); }

    // boolean xor
    // pow
    // instanceof

    Value& operator++() { return this->voidOperator(increment_function); }
    Value& operator--() { return this->voidOperator(decrement_function); }

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

    Value& operator!() { return this->unaryOperator(boolean_not_function); }
    Value& operator~() { return this->unaryOperator(bitwise_not_function); }

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

    zval* maybeDeref()
    {
        return this->isReference() ? Z_REFVAL(this->m_z) : &this->m_z;
    }

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

    template<typename T>
    static zval* paramHelper(const T& v, zval& z) { construct_zval(z, v); Z_TRY_DELREF(z); return &z; }

    template<typename Operator>
    Value& voidOperator(Operator op)
    {
        if (!this->isRefcounted()) {
            op(&this->m_z);
            return *this;
        }

        zval res;
        construct_zval(res, *this);
        op(&res);
        return this->assign(&res, AssignPolicy::AssignTemporary);
    }

    template<typename Operator>
    Value& unaryOperator(Operator op)
    {
        if (!this->isRefcounted()) {
            op(&this->m_z, &this->m_z);
            return *this;
        }

        zval res;
        op(&res, &this->m_z);
        return this->assign(&res, AssignPolicy::AssignTemporary);
    }

    template<typename Operator>
    Value& binaryOperator(Operator op, const Value& rhs)
    {
        if (!this->isRefcounted()) {
            op(&this->m_z, &this->m_z, &rhs.m_z);
            return *this;
        }

        zval res;
        op(&this->m_z, &this->m_z, &rhs.m_z);
        return this->assign(&res, AssignPolicy::AssignTemporary);
    }

    template<typename Operator>
    friend bool compareOperator(Operator op, const Value& lhs, const Value& rhs)
    {
        zval res;
        op(&res, const_cast<zval*>(&lhs.m_z), const_cast<zval*>(&rhs.m_z));
        return zend_is_true(&res);
    }

    friend void construct_zval(zval& z, const Value& v) { ZVAL_COPY(&z, &v.m_z); }
};

extern Value ErrorValue;

}

#endif /* PHPCXX */

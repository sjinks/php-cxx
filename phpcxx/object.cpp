#include <cassert>
#include "object.h"
#include "objectdimension.h"
#include "value.h"
#include "zendstring.h"

static inline zval* derefAndCheck(zval* z)
{
    if (Z_ISREF_P(z)) {
        z = Z_REFVAL_P(z);
    }

    if (UNEXPECTED(Z_TYPE_P(z) != IS_OBJECT)) {
        throw std::runtime_error("Not an object");
    }

    return z;
}

phpcxx::Object::Object()
{
    object_init(&this->m_z);
}

phpcxx::Object::Object(zval* z)
{
    if (Z_TYPE_P(z) == IS_OBJECT) {
        ZVAL_MAKE_REF(z);
        zend_reference* ref = Z_REF_P(z);
        ++GC_REFCOUNT(ref);
        ZVAL_REF(&this->m_z, ref);
    }
    else {
        ZVAL_COPY(&this->m_z, z);
        convert_to_object(&this->m_z);
    }
}

phpcxx::Object::Object(Value& v)
    : Object(v.pzval())
{
}

phpcxx::Object::Object(Object& other)
{
    ZVAL_COPY(&this->m_z, &other.m_z);
}

phpcxx::Object::Object(Object&& other)
{
    ZVAL_UNDEF(&this->m_z);
    std::swap(this->m_z, other.m_z);
}

phpcxx::Object::~Object()
{
    i_zval_ptr_dtor(&this->m_z ZEND_FILE_LINE_CC);
#ifdef PHPCXX_DEBUG
    ZVAL_UNDEF(&this->m_z);
#endif
}

phpcxx::Object& phpcxx::Object::operator=(const Object& other)
{
    phpcxx::assign(&this->m_z, &other.m_z);
    return *this;
}

phpcxx::ObjectDimension phpcxx::Object::operator[](const char* key)
{
    return this->operator[](ZendString(key).release());
}

phpcxx::ObjectDimension phpcxx::Object::operator[](const string& key)
{
    return this->operator[](ZendString(key).release());
}

phpcxx::ObjectDimension phpcxx::Object::operator[](const ZendString& key)
{
    return this->operator[](key.get());
}

phpcxx::ObjectDimension phpcxx::Object::operator[](zend_string* key)
{
    ObjectDimension::offset_t offset;
    offset.type  = ObjectDimension::OffsetType::String;
    offset.k.key = key;
    return ObjectDimension(&this->m_z, offset);
}

phpcxx::ObjectDimension phpcxx::Object::operator[](zend_ulong key)
{
    ObjectDimension::offset_t offset;
    offset.type = ObjectDimension::OffsetType::Numeric;
    offset.k.h  = key;
    return ObjectDimension(&this->m_z, offset);
}

bool phpcxx::Object::offsetExists(zend_long idx) const
{
    zval* z = derefAndCheck(&this->m_z);
    zval key;

    ZVAL_LONG(&key, idx);
    if (!Z_OBJ_HT_P(z)->has_dimension) {
        zend_error(E_NOTICE, "Trying to check element of non-array");
        return false;
    }

    int res = Z_OBJ_HT_P(z)->has_dimension(z, &key, 0);
    if (EG(exception)) {
        throw PhpException();
    }

    return res;
}

bool phpcxx::Object::offsetExists(const Value& key) const
{
    zval* z = derefAndCheck(&this->m_z);

    if (!Z_OBJ_HT_P(z)->has_dimension) {
        zend_error(E_NOTICE, "Trying to check element of non-array");
        return false;
    }

    int res = Z_OBJ_HT_P(z)->has_dimension(z, key.pzval(), 0);
    if (EG(exception)) {
        throw PhpException();
    }

    return res;
}

bool phpcxx::Object::offsetExists(zend_string* key) const
{
    zval* z = derefAndCheck(&this->m_z);
    zval k;

    ZVAL_STR(&k, key);
    if (!Z_OBJ_HT_P(z)->has_dimension) {
        zend_error(E_NOTICE, "Trying to check element of non-array");
        return false;
    }

    int res = Z_OBJ_HT_P(z)->has_dimension(z, &k, 0);
    if (EG(exception)) {
        throw PhpException();
    }

    return res;
}

phpcxx::Value phpcxx::Object::offsetGet(zend_long idx)
{
    Value key(idx);
    return this->offsetGet(key);
}

phpcxx::Value phpcxx::Object::offsetGet(const Value& key)
{
    zval* z = derefAndCheck(&this->m_z);
    zval result;

    if (Z_OBJ_HT_P(z)->read_dimension) {
        zval* retval = Z_OBJ_HT_P(z)->read_dimension(z, key.pzval(), BP_VAR_R, &result);
        if (!EG(exception)) {
            if (retval) {
                if (&result != retval) {
                    // If offsetExists returns false, retval will be &EG(uninitialized_zval), result will be garbage
                    ZVAL_COPY(&result, retval);
                }

                if (Z_ISREF(result)) {
                    Z_DELREF(result);
                }

                return Value(&result, CopyPolicy::Copy);
            }

            // This is the (unlikely) case when offsetExists returns IS_UNDEF
            return Value(nullptr);
        }
    }
    else {
        zend_throw_error(NULL, "Cannot use object as array");
    }

    assert(EG(exception));
    throw PhpException();
}

phpcxx::Value phpcxx::Object::offsetGet(zend_string* key)
{
    zval k;
    ZVAL_STR(&k, key);
    return this->offsetGet(Value(&k));
}

void phpcxx::Object::offsetSet(zend_long idx, const Value& v)
{
    Value key(idx);
    return this->offsetSet(key, v);
}

void phpcxx::Object::offsetSet(const Value& key, const Value& v)
{
    zval* z = derefAndCheck(&this->m_z);

    if (Z_OBJ_HT_P(z)->write_dimension) {
        Z_OBJ_HT_P(z)->write_dimension(z, key.pzval(), v.pzval());
    }
    else {
        zend_throw_error(NULL, "Cannot use object as array");
        ZEND_ASSUME(EG(exception));
    }

    if (EG(exception)) {
        throw PhpException();
    }
}

void phpcxx::Object::offsetSet(std::nullptr_t nullptr_t, const Value& v)
{
    Value key(nullptr);
    return this->offsetSet(key, v);
}

void phpcxx::Object::offsetSet(zend_string* key, const Value& v)
{
    zval k;
    ZVAL_STR(&k, key);
    return this->offsetSet(Value(&k), v);
}

void phpcxx::Object::offsetUnset(zend_long idx)
{
    Value key(idx);
    this->offsetUnset(key);
}

void phpcxx::Object::offsetUnset(const Value& key)
{
    zval* z = derefAndCheck(&this->m_z);

    if (Z_OBJ_HT_P(z)->unset_dimension) {
        Z_OBJ_HT_P(z)->unset_dimension(z, key.pzval());
    }
    else {
        zend_throw_error(NULL, "Cannot use object as array");
        ZEND_ASSUME(EG(exception));
    }

    if (EG(exception)) {
        throw PhpException();
    }
}

void phpcxx::Object::offsetUnset(zend_string* key)
{
    zval k;
    ZVAL_STR(&k, key);
    this->offsetUnset(Value(&k));
}

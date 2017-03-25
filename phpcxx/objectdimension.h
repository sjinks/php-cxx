#ifndef PHPCXX_OBJECTDIMENSION_H
#define PHPCXX_OBJECTDIMENSION_H

#include "phpcxx.h"
#include "value.h"

struct _zval_struct;

namespace phpcxx {

class PHPCXX_EXPORT ObjectDimension {
public:
    operator Value();

    ObjectDimension& operator=(const Value& v);

    ObjectDimension& operator+=(const Value& rhs)  { return this->operator=(this->operator Value() += rhs);  }
    ObjectDimension& operator-=(const Value& rhs)  { return this->operator=(this->operator Value() -= rhs);  }
    ObjectDimension& operator*=(const Value& rhs)  { return this->operator=(this->operator Value() *= rhs);  }
    ObjectDimension& operator/=(const Value& rhs)  { return this->operator=(this->operator Value() /= rhs);  }
    ObjectDimension& operator%=(const Value& rhs)  { return this->operator=(this->operator Value() %= rhs);  }
    ObjectDimension& operator^=(const Value& rhs)  { return this->operator=(this->operator Value() ^= rhs);  }
    ObjectDimension& operator&=(const Value& rhs)  { return this->operator=(this->operator Value() &= rhs);  }
    ObjectDimension& operator|=(const Value& rhs)  { return this->operator=(this->operator Value() |= rhs);  }
    ObjectDimension& operator<<=(const Value& rhs) { return this->operator=(this->operator Value() <<= rhs); }
    ObjectDimension& operator>>=(const Value& rhs) { return this->operator=(this->operator Value() >>= rhs); }

    Value value() { return this->operator Value(); }
private:
    enum class OffsetType : char { String, Numeric };
    struct offset_t {
        union {
            zend_string* key;
            zend_ulong h;
        } k;
        OffsetType type;
    };

    struct _zval_struct* m_object;
    offset_t m_offset;

private:
    friend class Object;

    ObjectDimension(struct _zval_struct* object, const offset_t& offset)
        : m_object(object), m_offset(offset)
    {}
};

}

#endif /* PHPCXX_OBJECTDIMENSION_H */

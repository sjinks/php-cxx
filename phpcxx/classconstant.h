#ifndef PHPCXX_CLASSCONSTANT_H
#define PHPCXX_CLASSCONSTANT_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend_long.h>
}

#include <type_traits>

namespace phpcxx {

class PHPCXX_EXPORT ClassConstant {
public:
    ClassConstant(const char* name);
    ClassConstant(const char* name, zend_long v);
    ClassConstant(const char* name, double v);
    ClassConstant(const char* name, bool v);
    ClassConstant(const char* name, const std::string& v);
    ClassConstant(const char* name, const char* v);
    ~ClassConstant();

    ClassConstant(ClassConstant&& other);
    ClassConstant(const ClassConstant& other);

    template<typename T, typename std::enable_if<(sizeof(T) < sizeof(zend_long)) && std::is_integral<T>::value>::type* = nullptr>
    ClassConstant(const char* name, T v) : ClassConstant(name, static_cast<zend_long>(v)) {}

    const char* name() const { return this->m_name;  }
    zval& value() const      { return this->m_value; }

private:
    const char* m_name;
    mutable zval m_value;
};

}

#endif /* PHPCXX_CLASSCONSTANT_H */

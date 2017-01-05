#ifndef PHPCXX_CONSTANT_H
#define PHPCXX_CONSTANT_H

#include "phpcxx.h"

#include <Zend/zend_long.h>
#include <memory>
#include <type_traits>

struct _zend_constant;

namespace phpcxx {

class ConstantPrivate;
class PHPCXX_EXPORT Constant {
public:
    Constant(const char* name);
    Constant(const char* name, zend_long v);
    Constant(const char* name, double v);
    Constant(const char* name, bool v);
    Constant(const char* name, const std::string& v);
    Constant(const char* name, const char* v);
    ~Constant();

    Constant(const Constant& other) : d_ptr(other.d_ptr) {}

    template<typename T, typename std::enable_if<(sizeof(T) < sizeof(zend_long)) && std::is_integral<T>::value>::type* = nullptr>
    Constant(const char* name, T v) : Constant(name, static_cast<zend_long>(v)) {}

    struct _zend_constant& get();
private:
    std::shared_ptr<ConstantPrivate> d_ptr;
};

}

#endif /* PHPCXX_CONSTANT_H */

#ifndef PHPCXX_VALUE_TCC
#define PHPCXX_VALUE_TCC

#ifndef PHPCXX_VALUE_H
#error "Please do not include this file directly, use value.h instead"
#endif

#include "fcall.h"

namespace phpcxx {

template<typename ...Args>
inline Value Value::operator()(Args&&... args)
{
    return call(this->pzval(), std::forward<Args>(args)...);
}

}

#endif /* PHPCXX_VALUE_TCC */

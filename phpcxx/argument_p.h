#ifndef PHPCXX_ARGUMENT_P_H
#define PHPCXX_ARGUMENT_P_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend_compile.h>
}

namespace phpcxx {

class PHPCXX_HIDDEN ArgumentPrivate {
    friend class Argument;
public:
    ArgumentPrivate(const char* argName, int type, const char* className, bool allowNull, bool byRef, bool isVariadic)
    {
        // argName can be nullptr; see ZEND_ARG_PASS_INFO macro
        this->m_arginfo.name              = argName;
        this->m_arginfo.class_name        = className;
        this->m_arginfo.type_hint         = static_cast<zend_uchar>(type);
        this->m_arginfo.allow_null        = allowNull;
        this->m_arginfo.pass_by_reference = byRef;
        this->m_arginfo.is_variadic       = isVariadic;
    }

    ArgumentPrivate(const zend_internal_arg_info& info)
        : m_arginfo(info)
    {}

private:
    zend_internal_arg_info m_arginfo;
};

}

#endif /* PHPCXX_ARGUMENT_P_H */

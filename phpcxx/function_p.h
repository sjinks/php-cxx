#ifndef PHPCXX_FUNCTION_P_H
#define PHPCXX_FUNCTION_P_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend_API.h>
#include <Zend/zend_compile.h>
}

#include <vector>
#include "argument.h"
#include "types.h"

namespace phpcxx {

class PHPCXX_HIDDEN FunctionPrivate {
    friend class Function;
    friend class Method;
public:
    FunctionPrivate(const char* name, InternalFunction c, zend_internal_arg_info* ai, uint32_t size)
    {
        this->m_arginfo.reserve(size);

        this->m_fe.fname    = name ? name : "";
        this->m_fe.handler  = c;
        this->m_fe.num_args = size - 1;
        this->m_fe.flags    = 0;

        std::memcpy(this->m_arginfo.data(), ai, size * sizeof(zend_internal_arg_info));
    }

    FunctionPrivate(const char* name, InternalFunction c, const Arguments& required, const Arguments& optional, bool byRef)
    {
        this->m_arginfo.reserve(required.size() + optional.size() + 1);

        this->m_fe.fname    = name ? name : "";
        this->m_fe.handler  = c;
        this->m_fe.num_args = static_cast<decltype(this->m_fe.num_args)>(required.size() + optional.size());
        this->m_fe.flags    = 0;

        zend_internal_arg_info f;
        f.name        = reinterpret_cast<const char*>(static_cast<zend_uintptr_t>(required.size()));
        f.class_name  = nullptr;
        f.type_hint   = IS_UNDEF;
        f.pass_by_reference = byRef;
        f.allow_null  = 0;
        f.is_variadic = 0;

        this->m_arginfo.push_back(f);

        for (auto&& arg : required) {
            this->m_arginfo.push_back(arg.get());
        }

        for (auto&& arg : optional) {
            this->m_arginfo.push_back(arg.get());
        }

        this->m_fe.arg_info = this->m_arginfo.data();
    }


private:
    zend_function_entry m_fe;
    std::vector<zend_internal_arg_info> m_arginfo;
};

}



#endif /* PHPCXX_FUNCTION_P_H */

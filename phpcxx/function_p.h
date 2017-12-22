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

    FunctionPrivate(const char* name, InternalFunction c, std::size_t nreq, const Arguments& args, bool byRef)
    {
        this->m_arginfo.reserve(args.size() + 1);

        this->m_fe.fname    = name ? name : "";
        this->m_fe.handler  = c;
        this->m_fe.num_args = static_cast<decltype(this->m_fe.num_args)>(args.size());
        this->m_fe.flags    = 0;

        zend_internal_arg_info f;
        f.name        = reinterpret_cast<const char*>(static_cast<zend_uintptr_t>(nreq));
        f.pass_by_reference = byRef;
        f.is_variadic = 0;
#if PHP_VERSION_ID < 70200
        f.class_name  = nullptr;
        f.type_hint   = IS_UNDEF;
        f.allow_null  = 0;
#else
        f.type        = 0;
#endif

        this->m_arginfo.push_back(f);

        for (auto&& arg : args) {
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

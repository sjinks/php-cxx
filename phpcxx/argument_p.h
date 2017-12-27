#ifndef PHPCXX_ARGUMENT_P_H
#define PHPCXX_ARGUMENT_P_H

#include "phpcxx.h"
#include <cstring>

extern "C" {
#include <Zend/zend_compile.h>
}

namespace phpcxx {

class PHPCXX_HIDDEN ArgumentPrivate {
    friend class Argument;
public:
    ArgumentPrivate(const char* argName, int type, const char* className, bool nullable, bool byRef, bool isVariadic);
    ArgumentPrivate(const ArgumentPrivate&) = delete;

    ArgumentPrivate(ArgumentPrivate&& other) noexcept
        : m_arginfo(std::move(other.m_arginfo)), m_class(other.m_class)
    {
        other.m_class = nullptr;
    }

    ArgumentPrivate(const zend_internal_arg_info& info)
        : m_arginfo(info), m_class(nullptr)
    {}

    ~ArgumentPrivate()
    {
        delete[] this->m_class;
    }

    void setType(zend_uchar type, bool nullable);
    void setClass(const char* name, bool nullable);
    void setByRef(bool v);
    void setVariadic(bool v);

    const char* name() const;
    const char* className() const;
    zend_uchar type() const;
    bool nullable() const;
    bool isPassedByReference() const;
    bool isVariadic() const;

    /**
     * @internal
     */
    char* internal_class() const
    {
        return this->m_class;
    }

    void clear_internal_class()
    {
        this->m_class = nullptr;
    }

private:
    zend_internal_arg_info m_arginfo;
    char* m_class;
};

}

#endif /* PHPCXX_ARGUMENT_P_H */

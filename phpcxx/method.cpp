#include "method.h"

phpcxx::Method::Method(const char* name, InternalFunction c, const Arguments& required, const Arguments& optional, bool byref)
    : Function(name, c, required, optional, byref)
{
    this->setAccess(Method::Public);
}

phpcxx::Method::Method(const Method& other)
    : Function(other)
{
}

phpcxx::Method&& phpcxx::Method::setAccess(Method::Access access)
{
    unsigned int flag       = static_cast<unsigned int>(access);
    this->d_ptr->m_fe.flags = (this->d_ptr->m_fe.flags & ~ZEND_ACC_PPP_MASK) | flag;
    return std::move(*this);
}


phpcxx::Method&& phpcxx::Method::setStatic(bool v)
{
    if (v) {
        this->d_ptr->m_fe.flags |= ZEND_ACC_STATIC;
    }
    else {
        this->d_ptr->m_fe.flags &= ~ZEND_ACC_STATIC;
    }

    return std::move(*this);
}


phpcxx::Method&& phpcxx::Method::setAbstract(bool v)
{
    if (v) {
        this->d_ptr->m_fe.flags |= ZEND_ACC_ABSTRACT;
    }
    else {
        this->d_ptr->m_fe.flags &= ~ZEND_ACC_ABSTRACT;
    }

    return std::move(*this);
}

phpcxx::Method&& phpcxx::Method::setFinal(bool v)
{
    if (v) {
        this->d_ptr->m_fe.flags |= ZEND_ACC_FINAL;
    }
    else {
        this->d_ptr->m_fe.flags &= ~ZEND_ACC_FINAL;
    }

    return std::move(*this);
}

#include "classbase.h"
#include "class_p.h"

phpcxx::ClassBase::ClassBase(const char* name, int flags)
    : d_ptr(std::make_shared<phpcxx::ClassPrivate>(this, name, flags))
{
}

phpcxx::ClassBase::~ClassBase()
{
}

void phpcxx::ClassBase::extends(zend_class_entry* ce)
{
    this->d_ptr->extends(ce);
}

void phpcxx::ClassBase::implements(zend_class_entry* ce)
{
    this->d_ptr->implements(ce);
}

struct _zend_class_entry* phpcxx::ClassBase::pce()
{
    return this->d_ptr->pce();
}

zend_class_entry* phpcxx::ClassBase::registerClass()
{
    if (!this->d_ptr->pce()) {
        this->d_ptr->initializeClass();
    }

    return this->d_ptr->pce();
}

phpcxx::Method phpcxx::ClassBase::addClassMethod(const Method& m)
{
    return this->d_ptr->addMethod(m);
}

void phpcxx::ClassBase::addClassConstant(const phpcxx::ClassConstant& c)
{
    this->d_ptr->addConstant(c);
}

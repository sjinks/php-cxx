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

void phpcxx::ClassBase::registerClass()
{
    this->d_ptr->initializeClass();
}

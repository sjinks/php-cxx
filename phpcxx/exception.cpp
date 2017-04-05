extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_API.h>
}

#include "exception.h"
#include "exception_p.h"

extern "C" {
zend_class_entry* phpcxx_exception_ce = nullptr;
}

int setup_phpcxx_exception()
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "PhpCxx\\Exception", nullptr);
    phpcxx_exception_ce = zend_register_internal_class_ex(&ce, zend_exception_get_default());
    return EXPECTED(phpcxx_exception_ce != nullptr) ? SUCCESS : FAILURE;
}

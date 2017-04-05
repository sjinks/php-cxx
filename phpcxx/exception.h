#ifndef PHPCXX_EXCEPTION_H
#define PHPCXX_EXCEPTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Zend/zend.h>
#include <Zend/zend_exceptions.h>

extern ZEND_DLEXPORT zend_class_entry* phpcxx_exception_ce;

#ifdef __cplusplus
}
#endif

#endif /* PHPCXX_EXCEPTION_H */

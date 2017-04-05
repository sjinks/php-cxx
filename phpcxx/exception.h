#ifndef PHPCXX_EXCEPTION_H
#define PHPCXX_EXCEPTION_H

extern "C" {
#include <Zend/zend_exceptions.h>
}

#ifdef __cplusplus
extern "C" {
#endif

extern ZEND_DLEXPORT zend_class_entry* phpcxx_exception_ce;

#ifdef __cplusplus
}
#endif

#endif /* PHPCXX_EXCEPTION_H */

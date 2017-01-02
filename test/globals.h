#ifndef TEST_GLOBALS_H
#define TEST_GLOBALS_H

#include "phpcxx/string.h"

void runPhpCode(const phpcxx::string& code);
void runPhpCode(const phpcxx::string& code, zval& retval);

#endif /* TEST_GLOBALS_H */

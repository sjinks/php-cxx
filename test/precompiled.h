#ifndef TEST_PRECOMPILED_H
#define TEST_PRECOMPILED_H

#include "phpcxx/phpcxx.h"
#include "phpcxx/value.h"
#include "phpcxx/argument.h"
#include "phpcxx/array.h"
#include "phpcxx/call.h"
#include "phpcxx/constant.h"
#include "phpcxx/emallocallocator.h"
#include "phpcxx/exception.h"
#include "phpcxx/function.h"
#include "phpcxx/functionhandler.h"
#include "phpcxx/helpers.h"
#include "phpcxx/map.h"
#include "phpcxx/module.h"
#include "phpcxx/objectdimension.h"
#include "phpcxx/object.h"
#include "phpcxx/operators.h"
#include "phpcxx/parameters.h"
#include "phpcxx/phpexception.h"
#include "phpcxx/string.h"
#include "phpcxx/superglobal.h"
#include "phpcxx/types.h"
#include "phpcxx/vector.h"
#include "phpcxx/zendstring.h"

#include <cstring>
#include <cstdlib>
#include <functional>
#include <memory>
#include <ostream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

extern "C" {
#   include <Zend/zend.h>
#   include <Zend/zend_compile.h>
#   include <Zend/zend_constants.h>
#   include <Zend/zend_exceptions.h>
#   include <main/SAPI.h>
#   include <main/php.h>
#   include <main/php_main.h>
#   include <main/php_variables.h>
}

#include "globals.h"
#include "testsapi.h"

#include "valgrind/valgrind.h"
#include "valgrind/memcheck.h"

#endif /* TEST_PRECOMPILED_H */

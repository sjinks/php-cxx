#ifndef PHPCXX_PRECOMPILED_H
#define PHPCXX_PRECOMPILED_H

#include "phpcxx.h"

extern "C" {
#   include <Zend/zend.h>
#   include <Zend/zend_API.h>
#   include <Zend/zend_alloc.h>
#   include <Zend/zend_compile.h>
#   include <Zend/zend_constants.h>
#   include <Zend/zend_exceptions.h>
#   include <Zend/zend_inheritance.h>
#   include <Zend/zend_long.h>
#   include <Zend/zend_modules.h>
#   include <Zend/zend_objects.h>
#   include <Zend/zend_objects_API.h>
#   include <Zend/zend_operators.h>
#   include <Zend/zend_string.h>
#   include <Zend/zend_types.h>
#   include <main/SAPI.h>
#   include <main/php.h>
#   include <main/php_variables.h>
}

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "argument.h"
#include "argument_p.h"
#include "array.h"
#include "bailoutrestorer.h"
#include "callable.h"
#include "class_p.h"
#include "classbase.h"
#include "constant.h"
#include "constant_p.h"
#include "emallocallocator.h"
#include "exception.h"
#include "exception_p.h"
#include "fcall.h"
#include "function.h"
#include "functionhandler.h"
#include "function_p.h"
#include "helpers.h"
#include "map.h"
#include "module.h"
#include "modulemap_p.h"
#include "module_p.h"
#include "objectdimension.h"
#include "object.h"
#include "operators.h"
#include "parameters.h"
#include "parameters_p.h"
#include "phpclass.h"
#include "phpexception.h"
#include "phpexception_p.h"
#include "string.h"
#include "superglobal.h"
#include "types.h"
#include "value.h"
#include "vector.h"
#include "zendstring.h"

#endif /* PHPCXX_PRECOMPILED_H */

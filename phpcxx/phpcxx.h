#ifndef PHPCXX_H
#define PHPCXX_H

extern "C" {
#   include <Zend/zend.h>
#   include <Zend/zend_API.h>
#   include <Zend/zend_alloc.h>
#   include <Zend/zend_exceptions.h>
#   include <Zend/zend_long.h>
#   include <Zend/zend_operators.h>
#   include <Zend/zend_string.h>
#   include <Zend/zend_types.h>
#   include <main/SAPI.h>
}

#ifdef PHPCXX_DEBUG
#include <iostream>

extern int phpcxx_ftrace;

#   define FTRACE()         do { if (::phpcxx_ftrace) { std::cout << "* " << __PRETTY_FUNCTION__ << std::endl; } } while (0)
#   define FTRACE_ENABLE()  do { ::phpcxx_ftrace = 1; } while (0)
#   define FTRACE_DISABLE() do { ::phpcxx_ftrace = 0; } while (0)
#   define FTRACE_ENABLED() (::phpcxx_ftrace != 0)
#else
#   define FTRACE()
#   define FTRACE_ENABLE()
#   define FTRACE_DISABLE()
#   define FTRACE_ENABLED   0
#endif

#if defined _WIN32 || defined __CYGWIN__
#   ifdef BUILDING_PHPCXX
#       if defined(__GNUC__) && __GNUC__ >= 4
#           define PHPCXX_EXPORT __attribute__((dllexport))
#       else
#           define PHPCXX_EXPORT __declspec(dllexport)
#       endif
#   else
#       if defined(__GNUC__) && __GNUC__ >= 4
#           define PHPCXX_EXPORT __attribute__((dllimport))
#       else
#           define PHPCXX_EXPORT __declspec(dllimport)
#       endif
#   endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHPCXX_EXPORT __attribute__((visibility("default")))
#   define PHPCXX_HIDDEN __attribute__((visibility("hidden")))
#else
#   define PHPCXX_EXPORT
#endif

#ifndef PHPCXX_HIDDEN
#   define PHPCXX_HIDDEN
#endif

#endif /* PHPCXX_H */

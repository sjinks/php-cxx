#ifndef PHPCXX_H
#define PHPCXX_H

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

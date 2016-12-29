#ifndef PHPCXX_STRING_H
#define PHPCXX_STRING_H

#include "phpcxx.h"

#include <string>
#include "emallocallocator.h"

namespace phpcxx {

using string = std::basic_string<char, std::char_traits<char>, EMallocAllocator<char> >;

}

namespace std {

static inline std::string to_string(const phpcxx::string& s)
{
    return std::string(s.c_str(), s.size());
}

}

#endif /* PHPCXX_STRING_H */

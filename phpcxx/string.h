#ifndef PHPCXX_STRING_H
#define PHPCXX_STRING_H

#include "phpcxx.h"

#include <string>
#include "emallocallocator.h"

namespace phpcxx {

/**
 * @brief `std::string` with `EMallocAllocator`
 * @see @ref EMallocAllocator
 */
using string = std::basic_string<char, std::char_traits<char>, EMallocAllocator<char> >;

}

namespace std {

/**
 * @brief Converts `phpcxx::string` to `std::string`
 * @param s `phpcxx::string`
 * @return A copy of `s` (`std::string`)
 */
static inline std::string to_string(const phpcxx::string& s)
{
    return std::string(s.c_str(), s.size());
}

}

#endif /* PHPCXX_STRING_H */

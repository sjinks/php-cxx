#ifndef PHPCXX_ARGUMENT_H
#define PHPCXX_ARGUMENT_H

#include "phpcxx.h"

#include <initializer_list>
#include <memory>
#include <ostream>
#include "types.h"

struct _zend_internal_arg_info;

namespace phpcxx {

class ArgumentPrivate;
class PHPCXX_EXPORT Argument {
public:
    Argument(const char* argName, phpcxx::ArgumentType type, bool allowNull = false, bool byRef = false, bool isVariadic = false);
    Argument(const char* argName, const char* className, bool allowNull = false, bool byRef = false, bool isVariadic = false);
    Argument(const struct _zend_internal_arg_info& other);

    Argument(const Argument&) = default;
    Argument(Argument&&) = default;
    ~Argument() = default;
    Argument& operator=(const Argument&) = default;
    Argument& operator=(Argument&&) = default;

    const char* name() const;
    const char* className() const;
    phpcxx::ArgumentType type() const;
    bool canBeNull() const;
    bool isPassedByReference() const;
    bool isVariadic() const;
    const struct _zend_internal_arg_info& get() const;

private:
    std::shared_ptr<ArgumentPrivate> d_ptr;
};

std::ostream& operator<<(std::ostream& os, const Argument& obj);

using Arguments = std::initializer_list<Argument>;

static inline Argument byValue(const char* name, phpcxx::ArgumentType type = phpcxx::ArgumentType::Any, bool canBeNull = true)
{
    return Argument(name, type, canBeNull, false);
}

static inline Argument byValue(const char* name, const char* className, bool canBeNull = true)
{
    return Argument(name, className, canBeNull, false);
}

static inline Argument byReference(const char* name, phpcxx::ArgumentType type = phpcxx::ArgumentType::Any, bool canBeNull = true)
{
    return Argument(name, type, canBeNull, true);
}

static inline Argument byReference(const char* name, const char* className, bool canBeNull = true)
{
    return Argument(name, className, canBeNull, true);
}

}

#endif /* PHPCXX_ARGUMENT_H */

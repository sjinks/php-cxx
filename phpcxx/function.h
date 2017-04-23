#ifndef PHPCXX_FUNCTION_H
#define PHPCXX_FUNCTION_H

#include "phpcxx.h"

#include <memory>
#include <vector>
#include "argument.h"
#include "functionhandler.h"
#include "types.h"

struct _zend_function_entry;
struct _zend_internal_arg_info;

namespace phpcxx {

class FunctionPrivate;
class PHPCXX_EXPORT Function {
public:
    [[gnu::nonnull]] Function(const char* name, InternalFunction c, size_t nreq = 0, const Arguments& args = {}, bool byref = false);
    Function(const Function& other);
    ~Function();

    Function&& addArgument(const Argument& arg);
    Function&& setNumberOfRequiredArguments(std::size_t n);
    Function&& setReturnByReference(bool byref);
    Function&& setAllowNull(bool allow);
    Function&& setTypeHint(ArgumentType t);
    Function&& setTypeHint(const char* className);
    Function&& setDeprecated(bool v);

    const struct _zend_function_entry& getFE() const;
    const std::vector<struct _zend_internal_arg_info>& getArgInfo() const;

protected:
    friend class FunctionPrivate;
    std::shared_ptr<FunctionPrivate> d_ptr;
};

template<FunctionPrototypeNN Handler>
static inline Function createFunction(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Function(name, &FunctionHandler::handler<Handler>, nreq, args, byRef);
}

template<FunctionPrototypeNV Handler>
static inline Function createFunction(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Function(name, &FunctionHandler::handler<Handler>, nreq, args, byRef);
}

template<FunctionPrototypeVN Handler>
static inline Function createFunction(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Function(name, &FunctionHandler::handler<Handler>, nreq, args, byRef);
}

template<FunctionPrototypeVV Handler>
static inline Function createFunction(const char* name, std::size_t nreq = 0, const Arguments& args = {}, bool byRef = false)
{
    return Function(name, &FunctionHandler::handler<Handler>, nreq, args, byRef);
}

}

#endif /* PHPCXX_FUNCTION_H */

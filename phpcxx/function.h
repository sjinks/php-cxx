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
    Function(const char* name, InternalFunction c, const Arguments& required = {}, const Arguments& optional = {}, bool byref = false);
    Function(const Function& other);
    ~Function();

    Function&& addRequiredArgument(const Argument& arg);
    Function&& addOptionalArgument(const Argument& arg);
    Function&& setReturnByReference(bool byref);
    Function&& setAllowNull(bool allow);
    Function&& setTypeHint(ArgumentType t);
    Function&& setTypeHint(const char* className);

    const struct _zend_function_entry& getFE() const;
    const std::vector<struct _zend_internal_arg_info>& getArgInfo() const;

private:
    friend class FunctionPrivate;
    std::shared_ptr<FunctionPrivate> d_ptr;
};

template<FunctionPrototypeNN Handler>
static inline Function createFunction(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Function(name, &FunctionHandler::handler<Handler>, required, optional, byRef);
}

template<FunctionPrototypeNV Handler>
static inline Function createFunction(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Function(name, &FunctionHandler::handler<Handler>, required, optional, byRef);
}

template<FunctionPrototypeVN Handler>
static inline Function createFunction(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Function(name, &FunctionHandler::handler<Handler>, required, optional, byRef);
}

template<FunctionPrototypeVV Handler>
static inline Function createFunction(const char* name, const Arguments& required = {}, const Arguments& optional = {}, bool byRef = false)
{
    return Function(name, &FunctionHandler::handler<Handler>, required, optional, byRef);
}

}

#endif /* PHPCXX_FUNCTION_H */

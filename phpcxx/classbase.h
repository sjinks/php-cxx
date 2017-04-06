#ifndef PHPCXX_CLASSBASE_H
#define PHPCXX_CLASSBASE_H

#include "phpcxx.h"

#include <memory>
#include <vector>
#include "classconstant.h"
#include "method.h"

namespace phpcxx {

/**
 * All classes that are going to be exposed to PHP
 * must be derived from this class
 */
class PHPCXX_EXPORT ClassImplementationBase {
public:
    virtual ~ClassImplementationBase()
    {
    }
};

class ClassPrivate;

/**
 * @internal
 */
class PHPCXX_EXPORT ClassBase {
public:
    [[gnu::nonnull]] ClassBase(const char* name, int flags);
    virtual ~ClassBase();

    ClassBase(ClassBase&& other) : d_ptr(std::move(other.d_ptr))
    {
    }

protected:
    virtual ClassImplementationBase* construct() = 0;

    virtual std::vector<Method> methods();
    virtual std::vector<ClassConstant> constants();

    void extends(zend_class_entry* ce);
    void implements(zend_class_entry* ce);

private:
    friend class ClassPrivate;
    friend class ModulePrivate;
    std::shared_ptr<ClassPrivate> d_ptr;

    void registerClass();
};

}

#endif /* PHPCXX_CLASSBASE_H */

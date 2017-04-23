#ifndef PHPCXX_CLASSBASE_H
#define PHPCXX_CLASSBASE_H

#include "phpcxx.h"

#include <memory>
#include <vector>
#include "method.h"

namespace phpcxx {

class ClassConstant;

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

    ClassBase(const ClassBase& other) : d_ptr(other.d_ptr)
    {
    }

    ClassBase(ClassBase&& other) : d_ptr(std::move(other.d_ptr))
    {
    }

    phpcxx::Method addClassMethod(const phpcxx::Method& m);
    void addClassConstant(const phpcxx::ClassConstant& c);

    void extends(zend_class_entry* ce);
    void implements(zend_class_entry* ce);

    struct _zend_class_entry* pce();
    struct _zend_class_entry* registerClass();

protected:
    virtual ClassImplementationBase* construct() { return nullptr; }

private:
    friend class ClassPrivate;
    friend class ModulePrivate;
    std::shared_ptr<ClassPrivate> d_ptr;
};

}

#endif /* PHPCXX_CLASSBASE_H */

#ifndef PHPCXX_CLASS_P_H
#define PHPCXX_CLASS_P_H

#include "phpcxx.h"

#include <memory>
#include <vector>
#include "method.h"

namespace phpcxx {

class ClassBase;

class PHPCXX_HIDDEN ClassPrivate {
public:
    [[gnu::nonnull]] ClassPrivate(ClassBase* const q, const char* name, int flags = 0)
        : q_ptr(q), m_name(name), m_this(nullptr), m_ce(nullptr),
          m_parent_ce(nullptr), m_interfaces(), m_flags(flags)
    {
    }

    ~ClassPrivate()
    {
        if (this->m_this) {
            zend_string_free(this->m_this);
            this->m_this = nullptr;
        }
    }

    void extends(zend_class_entry* ce)
    {
        this->m_parent_ce = ce;
    }

    [[gnu::nonnull]] void implements(zend_class_entry* ce)
    {
        this->m_interfaces.push_back(ce);
    }

    void initializeClass();

    zend_class_entry* pce()
    {
        return this->m_ce;
    }

private:
    ClassBase* const q_ptr;
    const char* m_name;
    zend_string* m_this;
    zend_class_entry* m_ce;
    zend_class_entry* m_parent_ce;
    std::vector<zend_class_entry*> m_interfaces;
    std::vector<phpcxx::Method> m_funcs;
    std::unique_ptr<zend_function_entry[]> m_zf;
    int m_flags;

    static zend_object* create_object(zend_class_entry* ce);
    static void free_object(zend_object* object);

    zend_function_entry* methods();
    void registerConstants();
};

}

#endif /* PHPCXX_CLASS_P_H */

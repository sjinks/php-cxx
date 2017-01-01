#include <cstring>
#include <stdexcept>
#include "extension.h"
#include "extension_p.h"

phpcxx::Extension::Extension(const char* name, const char* version)
    : d_ptr(new ExtensionPrivate(this, name, version))
{
}

phpcxx::Extension::~Extension() noexcept
{
}

struct _zend_module_entry* phpcxx::Extension::module()
{
    ExtensionPrivate* const d = this->d_ptr.get();
    d->m_fe.reserve(d->m_funcs.size() + 1);

    for (auto&& f : d->m_funcs) {
        d->m_fe.push_back(f.getFE());
    }

    zend_function_entry empty;
    std::memset(&empty, 0, sizeof(empty));
    d->m_fe.push_back(empty);

    d->entry.functions = d->m_fe.data();
    return &d->entry;
}

void phpcxx::Extension::registerExtension(phpcxx::Extension& other)
{
    int res = zend_startup_module(other.module());
    if (FAILURE == res) {
        throw std::runtime_error("Failed to register the extension");
    }
}

void phpcxx::Extension::onModuleInfo()
{
    display_ini_entries(&this->d_ptr->entry);
}

void phpcxx::Extension::addFunction(const Function& f)
{
    this->d_ptr->addFunction(f);
}

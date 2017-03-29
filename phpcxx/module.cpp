#include "constant.h"
#include "function.h"
#include "module.h"
#include "module_p.h"
#include "modulemap_p.h"

phpcxx::ModuleGlobals::ModuleGlobals()
{
    ModuleMap::instance().onGINIT();
}

phpcxx::ModuleGlobals::~ModuleGlobals()
{
    ModuleMap::instance().onGSHUTDOWN();
}

phpcxx::Module::Module(const char* name, const char* version)
    : d_ptr(new ModulePrivate(this, name, version))
{
}

phpcxx::Module::~Module()
{
}

struct _zend_module_entry* phpcxx::Module::module()
{
    return this->d_ptr->module();
}

phpcxx::ModuleGlobals* phpcxx::Module::globals()
{
    return this->d_ptr->globals();
}

bool phpcxx::Module::moduleStartup()
{
    return true;
}

bool phpcxx::Module::moduleShutdown()
{
    return true;
}

bool phpcxx::Module::requestStartup()
{
    return true;
}

bool phpcxx::Module::requestShutdown()
{
    return true;
}

void phpcxx::Module::moduleInfo()
{
    display_ini_entries(&this->d_ptr->entry);
}

void phpcxx::Module::registerModuleDependencies()
{
}

void phpcxx::Module::registerClasses()
{
}

void phpcxx::Module::registerIniEntries()
{
}

void phpcxx::Module::registerModules()
{
}

std::vector<phpcxx::Module*> phpcxx::Module::otherModules()
{
    return std::vector<phpcxx::Module*>();
}

std::vector<phpcxx::Function> phpcxx::Module::functions()
{
    return std::vector<phpcxx::Function>();
}

void phpcxx::Module::setGlobalsConstructor(globals_ctor_t f)
{
    if (UNEXPECTED(f == nullptr)) {
        throw std::logic_error("phpcxx::Module::setGlobalsConstructor: f cannot be nullptr");
    }

    this->d_ptr->module()->globals_ctor = reinterpret_cast<internal_globals_ctor_t>(f);
}

void phpcxx::Module::setGlobalsDestructor(globals_dtor_t f)
{
    if (UNEXPECTED(f == nullptr)) {
        throw std::logic_error("phpcxx::Module::setGlobalsDestructor: f cannot be nullptr");
    }

    this->d_ptr->module()->globals_dtor = reinterpret_cast<internal_globals_dtor_t>(f);
}

std::vector<phpcxx::Constant> phpcxx::Module::constants()
{
    return std::vector<phpcxx::Constant>();
}

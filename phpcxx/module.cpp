#include "constant.h"
#include "function.h"
#include "module.h"
#include "module_p.h"

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

phpcxx::ModuleGlobals* phpcxx::Module::globalsConstructor()
{
    return nullptr;
}

void phpcxx::Module::globalsDestructor(phpcxx::ModuleGlobals* g)
{
    delete g;
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

std::vector<phpcxx::Constant> phpcxx::Module::constants()
{
    return std::vector<phpcxx::Constant>();
}

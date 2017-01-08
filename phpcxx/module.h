#ifndef PHPCXX_MODULE_H
#define PHPCXX_MODULE_H

#include "phpcxx.h"

#include <memory>
#include <vector>

struct _zend_module_entry;

namespace phpcxx {

class Constant;
class Function;
class ModuleGlobals;
class ModulePrivate;

class PHPCXX_EXPORT Module {
public:
    [[gnu::nonnull(2 /* name */)]] Module(const char* name, const char* version);
    virtual ~Module();

    struct _zend_module_entry* module();
    ModuleGlobals* globals();

protected:
    virtual ModuleGlobals* globalsConstructor();
    virtual void globalsDestructor(ModuleGlobals* g);
    virtual bool moduleStartup();
    virtual bool moduleShutdown();
    virtual bool requestStartup();
    virtual bool requestShutdown();
    virtual void moduleInfo();

    void registerModuleDependencies();
    void registerClasses();
    void registerIniEntries();
    void registerModules();

    virtual std::vector<Module*> otherModules();
    virtual std::vector<Function> functions();
    virtual std::vector<Constant> constants();

private:
    friend class ModulePrivate;
    std::unique_ptr<ModulePrivate> d_ptr;
};

class PHPCXX_EXPORT ModuleGlobals {
public:
    virtual ~ModuleGlobals();
};

}

#endif /* PHPCXX_MODULE_H */

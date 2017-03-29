#ifndef PHPCXX_MODULE_H
#define PHPCXX_MODULE_H

#include "phpcxx.h"

#include <memory>
#include <vector>

struct _zend_module_entry;

namespace phpcxx { class ModuleGlobals; }

typedef struct _zend_phpcxx_globals {
    phpcxx::ModuleGlobals* globals;
} zend_phpcxx_globals;

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

    typedef void(*globals_ctor_t)(struct ::_zend_phpcxx_globals*);
    typedef void(*globals_dtor_t)(struct ::_zend_phpcxx_globals*);

protected:
    virtual bool moduleStartup();
    virtual bool moduleShutdown();
    virtual bool requestStartup();
    virtual bool requestShutdown();
    virtual void moduleInfo();

    [[gnu::nonnull]] void setGlobalsConstructor(globals_ctor_t f);
    [[gnu::nonnull]] void setGlobalsDestructor(globals_dtor_t f);

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

    typedef void(*internal_globals_ctor_t)(void*);
    typedef void(*internal_globals_dtor_t)(void*);
};

class PHPCXX_EXPORT ModuleGlobals {
public:
    ModuleGlobals();
    virtual ~ModuleGlobals();
};

}

#endif /* PHPCXX_MODULE_H */

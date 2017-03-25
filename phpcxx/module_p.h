#ifndef PHPCXX_MODULE_P_H
#define PHPCXX_MODULE_P_H

#include "phpcxx.h"

#include <main/php.h>
#include <Zend/zend.h>
#include <Zend/zend_modules.h>
#include <vector>

namespace phpcxx { class ModuleGlobals; }

ZEND_BEGIN_MODULE_GLOBALS(phpcxx)
    phpcxx::ModuleGlobals* globals;
ZEND_END_MODULE_GLOBALS(phpcxx)
extern ZEND_DECLARE_MODULE_GLOBALS(phpcxx);

namespace phpcxx {

class Function;
class Module;

class PHPCXX_HIDDEN ModulePrivate {
public:
    [[gnu::nonnull(2,3)]] ModulePrivate(Module* const q, const char* name, const char* version); // implicit first argument - `this`
    ~ModulePrivate();

    ModuleGlobals* globals()
    {
#ifdef ZTS
        return ZEND_TSRMG(phpcxx_globals_id, zend_phpcxx_globals*, globals);
#else
        return phpcxx_globals.globals;
#endif
    }

    zend_module_entry* module();

    zend_module_entry entry = {
        STANDARD_MODULE_HEADER_EX,
        nullptr,    // INI
        nullptr,    // deps
        nullptr,    // extname
        nullptr,    // functions,
        &ModulePrivate::moduleStartup,
        &ModulePrivate::moduleShutdown,
        &ModulePrivate::requestStartup,
        &ModulePrivate::requestShutdown,
        &ModulePrivate::moduleInfo,
        NO_VERSION_YET,
        PHP_MODULE_GLOBALS(phpcxx),
        nullptr,    // globalsInit,
        nullptr,    // globalsShutdown,
        nullptr,    // ZEND_MODULE_POST_ZEND_DEACTIVATE_N(phpcxx),
        STANDARD_MODULE_PROPERTIES_EX
    };

private:
    Module* const q_ptr;
    std::vector<Function> m_funcs;
    std::unique_ptr<zend_function_entry[]> m_zf;
/*
#ifdef ZTS
    ts_rsrc_id phpcxx_globals_id;
#else
    zend_phpcxx_globals phpcxx_globals;
#endif
*/
    static int moduleStartup(INIT_FUNC_ARGS);
    static int moduleShutdown(SHUTDOWN_FUNC_ARGS);
    static int requestStartup(INIT_FUNC_ARGS);
    static int requestShutdown(SHUTDOWN_FUNC_ARGS);
    static void moduleInfo(ZEND_MODULE_INFO_FUNC_ARGS);

    void registerOtherModules();
    void registerConstants();
};

}

#endif /* PHPCXX_MODULE_P_H */

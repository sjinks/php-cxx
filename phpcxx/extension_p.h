#ifndef PHPCXX_EXTENSION_P_H
#define PHPCXX_EXTENSION_P_H

#include "phpcxx.h"

#include <main/php.h>
#include <Zend/zend.h>
#include <Zend/zend_modules.h>
#include <vector>
#include "function.h"

ZEND_BEGIN_MODULE_GLOBALS(phpcxx)
ZEND_END_MODULE_GLOBALS(phpcxx)

extern ZEND_DECLARE_MODULE_GLOBALS(phpcxx);

PHP_MINIT_FUNCTION(phpcxx);
PHP_MSHUTDOWN_FUNCTION(phpcxx);
PHP_RINIT_FUNCTION(phpcxx);
PHP_RSHUTDOWN_FUNCTION(phpcxx);
PHP_MINFO_FUNCTION(phpcxx);
PHP_GINIT_FUNCTION(phpcxx);
PHP_GSHUTDOWN_FUNCTION(phpcxx);

namespace phpcxx {

class Extension;

class PHPCXX_HIDDEN ExtensionPrivate {
    friend class Extension;
public:
    ExtensionPrivate(Extension* const q, const char* name, const char* version);

    zend_module_entry entry = {
        STANDARD_MODULE_HEADER_EX,
        nullptr,    // INI
        nullptr,    // deps
        nullptr,    // extname
        nullptr,    // functions,
        &ExtensionPrivate::moduleStartup,
        &ExtensionPrivate::moduleShutdown,
        &ExtensionPrivate::requestStartup,
        &ExtensionPrivate::requestShutdown,
        &ExtensionPrivate::moduleInfo,
        NO_VERSION_YET,
        PHP_MODULE_GLOBALS(phpcxx),
        &ExtensionPrivate::globalsInit,
        &ExtensionPrivate::globalsShutdown,
        nullptr, // ZEND_MODULE_POST_ZEND_DEACTIVATE_N(phpcxx),
        STANDARD_MODULE_PROPERTIES_EX
    };

    void addFunction(const Function& f)
    {
        if (!this->entry.module_started) {
            this->m_funcs.push_back(f);
        }
        else {
            this->doRegisterFunction(f);
        }
    }

private:
    struct LateFunction {
        zend_function_entry fe;
        std::vector<zend_internal_arg_info> ai;
    };

private:
    Extension* const q_ptr;
    std::vector<Function> m_funcs;
    std::vector<zend_function_entry> m_fe;
    std::vector<std::unique_ptr<LateFunction> > m_late_functions;

    static int moduleStartup(INIT_FUNC_ARGS);
    static int moduleShutdown(SHUTDOWN_FUNC_ARGS);
    static int requestStartup(INIT_FUNC_ARGS);
    static int requestShutdown(SHUTDOWN_FUNC_ARGS);
    static void moduleInfo(ZEND_MODULE_INFO_FUNC_ARGS);
    static void globalsInit(void* g);
    static void globalsShutdown(void* g);

    void doRegisterFunction(const Function& f);
};

}

#endif /* PHPCXX_EXTENSION_P_H */

#include <cassert>
#include "constant.h"
#include "exception_p.h"
#include "function.h"
#include "module.h"
#include "module_p.h"
#include "modulemap_p.h"

#ifdef ZTS
    ts_rsrc_id phpcxx::ModulePrivate::phpcxx_globals_id;
#else
    zend_phpcxx_globals phpcxx::ModulePrivate::phpcxx_globals;
#endif

static void moduleNotFound(int number)
{
    zend_error(
        E_WARNING,
        "Unable to find the module by number %d. Probably phpcxx::Module instance has been destroyed too early",
        number
    );
}

phpcxx::ModulePrivate::ModulePrivate(phpcxx::Module* const q, const char* name, const char* version)
    : q_ptr(q)
{
    this->entry.name    = name;
    this->entry.version = version;

    ModuleMap::instance().add(this);
}

phpcxx::ModulePrivate::~ModulePrivate()
{
    ModuleMap::instance().remove(this->entry.module_number);
}

zend_module_entry* phpcxx::ModulePrivate::module()
{
    static zend_function_entry empty;
    assert(empty.fname == nullptr && empty.handler == nullptr && empty.arg_info == nullptr);

    if (!this->entry.functions) {
        this->m_funcs = this->q_ptr->functions();
        this->m_zf.reset(new zend_function_entry[this->m_funcs.size()+1]);
        zend_function_entry* ptr = this->m_zf.get();

        for (auto&& f : this->m_funcs) {
            *ptr++ = f.getFE();
        }

        *ptr = empty;
        this->entry.functions = this->m_zf.get();
    }

    return &this->entry;
}

int phpcxx::ModulePrivate::moduleStartup(INIT_FUNC_ARGS)
{
    zend_module_entry* me = EG(current_module);
    assert(me != nullptr);

    phpcxx::ModulePrivate* e = ModuleMap::instance().mapIdToModule(me->name, module_number);

    if (EXPECTED(e)) {
        if (FAILURE == setup_phpcxx_exception()) {
            return FAILURE;
        }

//        e->registerIniEntries();
//        e->registerClasses();
        e->registerConstants();
        e->registerOtherModules();

        try {
            return e->q_ptr->moduleStartup() ? SUCCESS : FAILURE;
        }
        catch (const std::exception& e) {
            zend_error(E_ERROR, "%s", e.what());
            return FAILURE;
        }
    }

    moduleNotFound(module_number);
    return SUCCESS;
}

int phpcxx::ModulePrivate::moduleShutdown(SHUTDOWN_FUNC_ARGS)
{
    zend_unregister_ini_entries(module_number);

    phpcxx::ModulePrivate* e = ModuleMap::instance().byId(module_number);
    ModuleMap::instance().remove(module_number);
    int retcode;

    if (EXPECTED(e)) {
        try {
            retcode = e->q_ptr->moduleShutdown() ? SUCCESS : FAILURE;
        }
        catch (const std::exception&) {
            retcode = FAILURE;
        }
    }
    else {
        moduleNotFound(module_number);
        retcode = SUCCESS;
    }

    return retcode;
}

int phpcxx::ModulePrivate::requestStartup(INIT_FUNC_ARGS)
{
    phpcxx::ModulePrivate* e = ModuleMap::instance().byId(module_number);

    if (EXPECTED(e)) {
        try {
            return e->q_ptr->requestStartup() ? SUCCESS : FAILURE;
        }
        catch (const std::exception& e) {
            zend_error(E_ERROR, "%s", e.what());
            return FAILURE;
        }
    }

    moduleNotFound(module_number);
    return SUCCESS;
}

int phpcxx::ModulePrivate::requestShutdown(SHUTDOWN_FUNC_ARGS)
{
    phpcxx::ModulePrivate* e = ModuleMap::instance().byId(module_number);

    if (EXPECTED(e)) {
        try {
            return e->q_ptr->requestShutdown() ? SUCCESS : FAILURE;
        }
        catch (const std::exception& e) {
            zend_error(E_ERROR, "%s", e.what());
            return FAILURE;
        }
    }

    moduleNotFound(module_number);
    return SUCCESS;
}

void phpcxx::ModulePrivate::moduleInfo(ZEND_MODULE_INFO_FUNC_ARGS)
{
    phpcxx::ModulePrivate* e = ModuleMap::instance().byId(zend_module->module_number);
    if (EXPECTED(e)) {
        e->q_ptr->moduleInfo();
    }
    else {
        moduleNotFound(zend_module->module_number);
    }
}

void phpcxx::ModulePrivate::registerOtherModules()
{
    std::vector<Module*> others = this->q_ptr->otherModules();
    for (auto&& m : others) {
        /// TODO check if return value is SUCCESS
        zend_startup_module(m->module());
    }
}

void phpcxx::ModulePrivate::registerConstants()
{
    std::vector<phpcxx::Constant> constants = this->q_ptr->constants();
    for (auto&& c : constants) {
        zend_constant& zc = c.get();
        zc.module_number  = this->entry.module_number;
        if (FAILURE == zend_register_constant(&zc)) {
            // Zend calls zend_string_release() for constant name upon failure
            zval_ptr_dtor(&zc.value);
        }
    }
}

void phpcxx::ModulePrivate::globalsInit(void* g)
{
    assert(g != nullptr);

    zend_phpcxx_globals* mg = reinterpret_cast<zend_phpcxx_globals*>(g);
    mg->globals = nullptr;
}

void phpcxx::ModulePrivate::globalsShutdown(void* g)
{
    assert(g != nullptr);

    zend_phpcxx_globals* mg = reinterpret_cast<zend_phpcxx_globals*>(g);
    delete mg->globals;
    mg->globals = nullptr;
}

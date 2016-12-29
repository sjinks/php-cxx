#include <cassert>
#include <stdexcept>
#include <unordered_map>

#include "extension.h"
#include "extension_p.h"

#ifdef ZTS
#include <mutex>
#endif

ZEND_DECLARE_MODULE_GLOBALS(phpcxx);

class maps_t {
private:
#ifdef ZTS
    std::mutex                                          g_mutex;
#endif
    std::unordered_map<std::string, phpcxx::Extension*> g_name2ext;
    std::unordered_map<int, phpcxx::Extension*>         g_id2ext;
public:
    phpcxx::Extension* extByModNumber(int module)
    {
#ifdef ZTS
        std::unique_lock<std::mutex> locker(this->g_mutex);
#endif

        auto i2e_it = this->g_id2ext.find(module);
        if (EXPECTED(i2e_it != this->g_id2ext.end())) {
            return i2e_it->second;
        }

        return nullptr;
    }

    void addExtension(const char* name, phpcxx::Extension* ext)
    {
#ifdef ZTS
        std::unique_lock<std::mutex> locker(this->g_mutex);
#endif
        this->g_name2ext.emplace(std::string(name), ext);
    }

    phpcxx::Extension* mapNumberToExt(const char* name, int number)
    {
#ifdef ZTS
        std::unique_lock<std::mutex> locker(this->g_mutex);
#endif

        auto n2e_it = this->g_name2ext.find(name);
        if (EXPECTED(n2e_it != this->g_name2ext.end())) {
            phpcxx::Extension* e = n2e_it->second;
            this->g_id2ext.emplace(number, e);
            this->g_name2ext.erase(n2e_it);
            return e;
        }

        return nullptr;
    }
};

static maps_t& maps()
{
    static maps_t result;
    return result;
}

phpcxx::ExtensionPrivate::ExtensionPrivate(phpcxx::Extension* const q, const char* name, const char* version)
    : q_ptr(q)
{
    if (!name) {
        throw std::logic_error("Extension name not specified");
    }

    this->entry.name    = name;
    this->entry.version = version;

    maps().addExtension(name, q);
}

int phpcxx::ExtensionPrivate::moduleStartup(INIT_FUNC_ARGS)
{
    zend_module_entry* me = EG(current_module);
    assert(me != nullptr);

    phpcxx::Extension* e = maps().mapNumberToExt(me->name, module_number);

//  const zend_ini_entry_def* ini;
//  zend_register_ini_entries(ini, int module_number)

    if (EXPECTED(e)) {
        try {
            e->onModuleStartup();
            return SUCCESS;
        }
        catch (const std::exception& e) {
            zend_error(E_ERROR, "%s", e.what());
            return FAILURE;
        }
    }

    zend_error(E_WARNING, "Unable to find the module by number %d. Probably phpcxx::Extension instance has been destroyed too early", module_number);
    return SUCCESS;
}

int phpcxx::ExtensionPrivate::moduleShutdown(SHUTDOWN_FUNC_ARGS)
{
    zend_unregister_ini_entries(module_number);

    phpcxx::Extension* e = maps().extByModNumber(module_number);
    if (EXPECTED(e)) {
        try {
            e->onModuleShutdown();
            return SUCCESS;
        }
        catch (const std::exception& e) {
            zend_error(E_ERROR, "%s", e.what());
            return FAILURE;
        }
    }

    zend_error(E_WARNING, "Unable to find the module by number %d. Probably phpcxx::Extension instance has been destroyed too early", module_number);
    return SUCCESS;
}

int phpcxx::ExtensionPrivate::requestStartup(INIT_FUNC_ARGS)
{
    phpcxx::Extension* e = maps().extByModNumber(module_number);
    if (EXPECTED(e)) {
        try {
            e->onRequestStartup();
            return SUCCESS;
        }
        catch (const std::exception& e) {
            zend_error(E_ERROR, "%s", e.what());
            return FAILURE;
        }
    }

    zend_error(E_WARNING, "Unable to find the module by number %d. Probably phpcxx::Extension instance has been destroyed too early", module_number);
    return SUCCESS;
}

int phpcxx::ExtensionPrivate::requestShutdown(SHUTDOWN_FUNC_ARGS)
{
    phpcxx::Extension* e = maps().extByModNumber(module_number);
    if (EXPECTED(e)) {
        try {
            e->onRequestShutdown();
            return SUCCESS;
        }
        catch (const std::exception& e) {
            zend_error(E_ERROR, "%s", e.what());
            return FAILURE;
        }
    }

    zend_error(E_WARNING, "Unable to find the module by number %d. Probably phpcxx::Extension instance has been destroyed too early", module_number);
    return SUCCESS;
}

void phpcxx::ExtensionPrivate::moduleInfo(ZEND_MODULE_INFO_FUNC_ARGS)
{
    phpcxx::Extension* e = maps().extByModNumber(zend_module->module_number);
    if (EXPECTED(e)) {
        e->onModuleInfo();
    }
    else {
        zend_error(E_WARNING, "Unable to find the module by number %d. Probably phpcxx::Extension instance has been destroyed too early", zend_module->module_number);
    }
}

void phpcxx::ExtensionPrivate::globalsInit(void* g)
{
}

void phpcxx::ExtensionPrivate::globalsShutdown(void* g)
{
}

#include <cassert>
#include <stdexcept>
#include <unordered_map>

#include "extension.h"
#include "extension_p.h"

#ifdef ZTS
#include <mutex>
#endif

ZEND_DECLARE_MODULE_GLOBALS(phpcxx);

class ExtensionMap {
private:
#ifdef ZTS
    std::mutex                                          g_mutex;        ///< Mutex to synchronize access to the maps
#endif
    std::unordered_map<std::string, phpcxx::Extension*> g_name2ext;     ///< Maps extension name to Extension
    std::unordered_map<int, phpcxx::Extension*>         g_id2ext;       ///< Maps extension number to Extension

    ExtensionMap() {}
    ExtensionMap(const ExtensionMap&) = delete;
    ExtensionMap& operator=(const ExtensionMap&) = delete;

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

    bool extensionExists(const char* name)
    {
#ifdef ZTS
        std::unique_lock<std::mutex> locker(this->g_mutex);
#endif
        return this->g_name2ext.count(std::string(name)) != 0;
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
            // this->g_name2ext.erase(n2e_it);
            return e;
        }

        return nullptr;
    }

    void clear()
    {
#ifdef ZTS
        std::unique_lock<std::mutex> locker(this->g_mutex);
#endif
        this->g_id2ext.clear();
        this->g_name2ext.clear();
    }

    /**
     * Ensures that ExtensionMap is initialized when we call this function.
     * If we declare ExtensionMap as a global static, we will depend on the
     * initialization order of static globals, which, in turn, depends on
     * the order of object files during the link time. With static
     * ExtensionMap we encountered strange floating point exceptions
     * happening inside std::unordered_map - possibly because emplace()
     * happened before the map was really initialized.
     */
    static ExtensionMap& instance()
    {
        static ExtensionMap self;
        return self;
    }
};

static void complainExtensionNotFound(int number)
{
    zend_error(
        E_WARNING,
        "Unable to find the module by number %d. Probably phpcxx::Extension instance has been destroyed too early",
        number
    );
}

phpcxx::ExtensionPrivate::ExtensionPrivate(phpcxx::Extension* const q, const char* name, const char* version)
    : q_ptr(q)
{
    if (!name) {
        throw std::logic_error("Extension name not specified");
    }

    if (ExtensionMap::instance().extensionExists(name)) {
        throw std::logic_error("This extension has already been registered - " + std::string(name));
    }

    this->entry.name    = name;
    this->entry.version = version;

    ExtensionMap::instance().addExtension(name, q);
}

int phpcxx::ExtensionPrivate::moduleStartup(INIT_FUNC_ARGS)
{
    zend_module_entry* me = EG(current_module);
    assert(me != nullptr);

    phpcxx::Extension* e = ExtensionMap::instance().mapNumberToExt(me->name, module_number);

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

    complainExtensionNotFound(module_number);
    return SUCCESS;
}

int phpcxx::ExtensionPrivate::moduleShutdown(SHUTDOWN_FUNC_ARGS)
{
    zend_unregister_ini_entries(module_number);

    phpcxx::Extension* e = ExtensionMap::instance().extByModNumber(module_number);
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

    complainExtensionNotFound(module_number);
    return SUCCESS;
}

int phpcxx::ExtensionPrivate::requestStartup(INIT_FUNC_ARGS)
{
    phpcxx::Extension* e = ExtensionMap::instance().extByModNumber(module_number);
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

    complainExtensionNotFound(module_number);
    return SUCCESS;
}

int phpcxx::ExtensionPrivate::requestShutdown(SHUTDOWN_FUNC_ARGS)
{
    phpcxx::Extension* e = ExtensionMap::instance().extByModNumber(module_number);
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

    complainExtensionNotFound(module_number);
    return SUCCESS;
}

void phpcxx::ExtensionPrivate::moduleInfo(ZEND_MODULE_INFO_FUNC_ARGS)
{
    phpcxx::Extension* e = ExtensionMap::instance().extByModNumber(zend_module->module_number);
    if (EXPECTED(e)) {
        e->onModuleInfo();
    }
    else {
        complainExtensionNotFound(zend_module->module_number);
    }
}

void phpcxx::ExtensionPrivate::globalsInit(void* g)
{
}

void phpcxx::ExtensionPrivate::globalsShutdown(void* g)
{
    ExtensionMap::instance().clear();
}

void phpcxx::ExtensionPrivate::doRegisterFunction(const Function& f)
{
    std::unique_ptr<ExtensionPrivate::LateFunction> late(new ExtensionPrivate::LateFunction);

    late->fe = f.getFE();
    late->ai = f.getArgInfo();

    zend_register_functions(nullptr, &late->fe, nullptr, MODULE_PERSISTENT);
    this->m_late_functions.push_back(std::move(late));
}

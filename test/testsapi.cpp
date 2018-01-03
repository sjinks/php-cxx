#include <cstring>
#include <vector>
#include "phpcxx/module.h"
#include "phpcxx/zendstring.h"
#include "testsapi.h"

extern "C" {
#include <main/php.h>
#include <main/php_main.h>
#include <main/SAPI.h>
#include <main/php_variables.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_ini.h>
#include <Zend/zend_string.h>
#include <sapi/embed/php_embed.h>
}

namespace {

static PHP_MINIT_FUNCTION(fake)
{
    PG(log_errors)        = 1;
    PG(display_errors)    = 0;
    EG(error_reporting)   = E_ALL;
    PG(report_zend_debug) = 0;
    return SUCCESS;
}

static zend_module_entry fake_module = {
    STANDARD_MODULE_HEADER_EX,
    nullptr,    // INI
    nullptr,    // deps
    const_cast<char*>("fake"),    // extname
    nullptr,    // functions,
    PHP_MINIT(fake),
    nullptr,    // MSHUTDOWN
    nullptr,    // RINIT
    nullptr,    // RSHUTDOWN
    nullptr,    // MINFO
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};

static int my_sapi_startup(sapi_module_struct* sapi_module);
static int my_sapi_activate();
static int my_sapi_deactivate();
static size_t my_sapi_ub_write(const char* str, size_t str_length);
static void my_sapi_flush(void*);
static void my_sapi_send_header(sapi_header_struct*, void*);
static char* my_sapi_read_cookies();
#if PHP_VERSION_ID >= 70100
static void my_sapi_log_message(char* message, int syslog_type_int);
#else
static void my_sapi_log_message(char* message);
#endif

class SAPIGlobals {
public:
    std::ostream* out;
    std::ostream* err;
    std::vector<zend_module_entry> mods;

    static SAPIGlobals& instance()
    {
        static SAPIGlobals self;
        return self;
    }

private:
    SAPIGlobals() : out(nullptr), err(nullptr)
    {
        php_embed_module.startup      = my_sapi_startup;
        php_embed_module.activate     = my_sapi_activate;
        php_embed_module.deactivate   = my_sapi_deactivate;
        php_embed_module.ub_write     = my_sapi_ub_write;
        php_embed_module.flush        = my_sapi_flush;
        php_embed_module.send_header  = my_sapi_send_header;
        php_embed_module.read_cookies = my_sapi_read_cookies;
        php_embed_module.log_message  = my_sapi_log_message;

        mods.push_back(fake_module);
    }
};

static int my_sapi_startup(sapi_module_struct* sapi_module)
{
    SAPIGlobals& g = SAPIGlobals::instance();
    return php_module_startup(sapi_module, g.mods.data(), static_cast<uint>(g.mods.size()));
}

static int my_sapi_activate()
{
    return SUCCESS;
}

static int my_sapi_deactivate()
{
    my_sapi_flush(nullptr);
    return SUCCESS;
}

static size_t my_sapi_ub_write(const char* str, size_t str_length)
{
    SAPIGlobals::instance().out->write(str, static_cast<std::streamsize>(str_length)).flush();
    return str_length;
}

static void my_sapi_flush(void*)
{
    SAPIGlobals::instance().out->flush();
}

static void my_sapi_send_header(sapi_header_struct*, void*)
{
}

static char* my_sapi_read_cookies()
{
    return nullptr;
}

#if PHP_VERSION_ID >= 70100
static void my_sapi_log_message(char* message, int syslog_type_int)
{
    *(SAPIGlobals::instance().err) << message << std::endl;
}
#else
static void my_sapi_log_message(char* message)
{
    *(SAPIGlobals::instance().err) << message << std::endl;
}
#endif

} // namespace

TestSAPI::TestSAPI(std::ostream& out, std::ostream& err)
    : m_initialized(false), m_in_request(false)
{
    SAPIGlobals& g = SAPIGlobals::instance();
    g.out = &out;
    g.err = &err;
}

TestSAPI::~TestSAPI()
{
    if (this->m_initialized) {
        if (!this->m_in_request) {
            // php_embed_shutdown() calls php_request_shutdown(), we need to start a request
            php_request_startup();
        }

        php_embed_shutdown();
    }

    SAPIGlobals& g = SAPIGlobals::instance();
    g.mods.clear();
    g.mods.push_back(fake_module);
}

void TestSAPI::addModule(phpcxx::Module& ext)
{
    if (!this->m_initialized) {
        SAPIGlobals& g = SAPIGlobals::instance();
        g.mods.push_back(*ext.module());
    }
}

void TestSAPI::run(std::function<void(void)> callback)
{
    if (!this->m_initialized) {
        php_embed_init(0, nullptr);
        this->m_initialized = true;
    }
    else {
        if (this->m_in_request) {
            php_request_shutdown(nullptr);
        }

        php_request_startup();
    }

    this->m_in_request = true;

    zend_first_try {
        callback();
    }
    zend_catch {
    }
    zend_end_try();

    if (EG(exception) && EG(current_execute_data) && EG(current_execute_data)->opline) {
        zend_clear_exception();
    }
    else {
        if (EG(prev_exception)) {
            OBJ_RELEASE(EG(prev_exception));
            EG(prev_exception) = nullptr;
        }

        if (EG(exception)) {
            OBJ_RELEASE(EG(exception));
            EG(exception) = nullptr;
        }
    }

//    php_request_shutdown(nullptr);
}

void TestSAPI::setOutputStream(std::ostream* os)
{
    SAPIGlobals::instance().out = os;
}

void TestSAPI::setErrorStream(std::ostream* os)
{
    SAPIGlobals::instance().err = os;
}

#include <cstring>
#include <vector>
#include "phpcxx/module.h"
#include "testsapi.h"

extern "C" {
#include <main/php.h>
#include <main/php_main.h>
#include <main/SAPI.h>
#include <main/php_variables.h>
#include <Zend/zend_exceptions.h>
}

namespace {

static const char hardcoded_ini[] =
    "html_errors=0\n"
    "log_errors=1\n"
    "display_errors=0\n"
    "error_reporting=E_ALL\n"
    "report_zend_debug=0\n"
    "register_argc_argv=1\n"
    "implicit_flush=1\n"
    "output_buffering=0\n"
    "max_execution_time=0\n"
    "extension_dir=\n"
    "max_input_time=-1\n\0"
;

static int sapi_startup(sapi_module_struct* sapi_module);
static int sapi_activate();
static int sapi_deactivate();
static size_t sapi_ub_write(const char* str, size_t str_length);
static void sapi_flush(void*);
static void sapi_send_header(sapi_header_struct*, void*);
static char* sapi_read_cookies();
#if PHP_VERSION_ID >= 70100
static void sapi_log_message(char* message, int syslog_type_int);
#else
static void sapi_log_message(char* message);
#endif

class SAPIGlobals {
public:
    std::ostream* out;
    std::ostream* err;
    std::vector<zend_module_entry> mods;

    sapi_module_struct sapi = {
        const_cast<char*>("test"),          // name
        const_cast<char*>("Test SAPI"),     // pretty_name
        sapi_startup,                       // startup
        php_module_shutdown_wrapper,        // shutdown
        sapi_activate,                      // activate
        sapi_deactivate,                    // deactivate
        sapi_ub_write,                      // ub_write
        sapi_flush,                         // flush
#if PHP_VERSION_ID >= 70100
        nullptr,                            // get_stat
#else
        nullptr,                            // get UID
#endif
        nullptr,                            // getenv
        zend_error,                         // sapi_error
        nullptr,                            // header_handler
        nullptr,                            // send_headers
        sapi_send_header,                   // send_header
        nullptr,                            // read_post
        sapi_read_cookies,                  // read_cookies
        php_import_environment_variables,   // register_server_variables
        sapi_log_message,                   // log_message
        nullptr,                            // double (*get_request_time)(void);
        nullptr,                            // void (*terminate_process)(void);
        nullptr,                            // char *php_ini_path_override;
#if PHP_VERSION_ID < 70100
        nullptr,                            // void (*block_interruptions)(void);
        nullptr,                            // void (*unblock_interruptions)(void);
#endif
        nullptr,    // void (*default_post_reader)(void);
        nullptr,    // void (*treat_data)(int arg, char *str, zval *destArray);
        const_cast<char*>("-"), // char *executable_location;
        1,          // php_ini_ignore
        1,          // php_ini_ignore_cwd
        nullptr,    // int (*get_fd)(int *fd);
        nullptr,    // int (*force_http_10)(void);
        nullptr,    // int (*get_target_uid)(uid_t *);
        nullptr,    // int (*get_target_gid)(gid_t *);
        nullptr,    // unsigned int (*input_filter)(int arg, char *var, char **val, size_t val_len, size_t *new_val_len);
        nullptr,    // void (*ini_defaults)(HashTable *configuration_hash);
        1,          // phpinfo_as_text
        nullptr,    // ini
        nullptr,    // const zend_function_entry *additional_functions
        nullptr     // unsigned int (*input_filter_init)(void)
    };

    static SAPIGlobals& instance()
    {
        static SAPIGlobals self;
        return self;
    }

private:
    SAPIGlobals() : out(nullptr), err(nullptr) {}
};

static int sapi_startup(sapi_module_struct* sapi_module)
{
    SAPIGlobals& g = SAPIGlobals::instance();
    return php_module_startup(sapi_module, g.mods.data(), static_cast<uint>(g.mods.size()));
}

static int sapi_activate()
{
    return SUCCESS;
}

static int sapi_deactivate()
{
    sapi_flush(nullptr);
    return SUCCESS;
}

static size_t sapi_ub_write(const char* str, size_t str_length)
{
    SAPIGlobals::instance().out->write(str, static_cast<std::streamsize>(str_length)).flush();
    return str_length;
}

static void sapi_flush(void*)
{
    SAPIGlobals::instance().out->flush();
}

static void sapi_send_header(sapi_header_struct*, void*)
{
}

static char* sapi_read_cookies()
{
    return nullptr;
}

#if PHP_VERSION_ID >= 70100
static void sapi_log_message(char* message, int syslog_type_int)
{
    *(SAPIGlobals::instance().err) << message << std::endl;
}
#else
static void sapi_log_message(char* message)
{
    *(SAPIGlobals::instance().err) << message << std::endl;
}
#endif
}

TestSAPI::TestSAPI(std::ostream& out, std::ostream& err)
    : m_initialized(false)
{
#ifdef ZTS
    tsrm_startup(1, 1, 0, NULL);
    ts_resource(0);
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

#if defined(ZEND_SIGNALS)
    zend_signal_startup();
#endif

    SAPIGlobals& g = SAPIGlobals::instance();
    g.out = &out;
    g.err = &err;

    ::sapi_startup(&g.sapi);
}

TestSAPI::~TestSAPI()
{
    php_module_shutdown();
    ::sapi_shutdown();

    SAPIGlobals& g = SAPIGlobals::instance();
    g.mods.clear();

#ifdef ZTS
    tsrm_shutdown();
#endif

    if (g.sapi.ini_entries) {
        delete[] g.sapi.ini_entries;
        g.sapi.ini_entries = nullptr;
    }
}

void TestSAPI::initialize()
{
    if (!this->m_initialized) {
        SAPIGlobals& g = SAPIGlobals::instance();

        g.sapi.ini_entries = new char[sizeof(hardcoded_ini)];
        std::memcpy(g.sapi.ini_entries, hardcoded_ini, sizeof(hardcoded_ini));

        g.sapi.startup(&g.sapi);
        this->m_initialized = true;
    }
}

void TestSAPI::addModule(phpcxx::Module& ext)
{
    if (!this->m_initialized) {
        SAPIGlobals& g = SAPIGlobals::instance();
        g.mods.push_back(*ext.module());
    }
}

void TestSAPI::run()
{
    this->initialize();

    SG(options) |= SAPI_OPTION_NO_CHDIR;
    php_request_startup();
    php_request_shutdown(nullptr);
}

void TestSAPI::run(std::function<void(void)> callback)
{
    this->initialize();

    SG(options) |= SAPI_OPTION_NO_CHDIR;
    php_request_startup();
    SG(headers_sent) = 1;
    SG(request_info).no_headers = 1;
    php_register_variable(const_cast<char*>("PHP_SELF"), const_cast<char*>("-"), NULL);

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

    php_request_shutdown(nullptr);
}

void TestSAPI::setOutputStream(std::ostream* os)
{
    SAPIGlobals::instance().out = os;
}

void TestSAPI::setErrorStream(std::ostream* os)
{
    SAPIGlobals::instance().err = os;
}

#include <main/php.h>
#include <main/php_main.h>
#include <main/SAPI.h>
#include <main/php_variables.h>
#include <Zend/zend_exceptions.h>
#include <vector>
#include "phpcxx/extension.h"
#include "testsapi.h"

namespace {
static int sapi_startup(sapi_module_struct* sapi_module);
static int sapi_activate();
static int sapi_deactivate();
static size_t sapi_ub_write(const char* str, size_t str_length);
static void sapi_flush(void*);
static void sapi_send_header(sapi_header_struct*, void*);
static char* sapi_read_cookies();
static void sapi_log_message(char* message);

class SAPIGlobals {
public:
    std::ostream* out;
    std::ostream* err;
    std::vector<zend_module_entry> exts;

    sapi_module_struct sapi = {
        (char*)"test",
        (char*)"Test SAPI",
        sapi_startup,
        php_module_shutdown_wrapper,
        sapi_activate,
        sapi_deactivate,
        sapi_ub_write,
        sapi_flush,
        nullptr,    // get UID
        nullptr,    // get env
        zend_error,
        nullptr,    // header handler
        nullptr,    // send headers handler
        sapi_send_header,
        nullptr,    // read POST data
        sapi_read_cookies,
        php_import_environment_variables,
        sapi_log_message,
        nullptr,    // double (*get_request_time)(void);
        nullptr,    // void (*terminate_process)(void);
        nullptr,    // char *php_ini_path_override;
        nullptr,    // void (*block_interruptions)(void);
        nullptr,    // void (*unblock_interruptions)(void);
        nullptr,    // void (*default_post_reader)(void);
        nullptr,    // void (*treat_data)(int arg, char *str, zval *destArray);
        (char*)"-", // char *executable_location;
        0,          // php_ini_ignore
        1,          // php_ini_ignore_cwd
        nullptr,    // int (*get_fd)(int *fd);
        nullptr,    // int (*force_http_10)(void);
        nullptr,    // int (*get_target_uid)(uid_t *);
        nullptr,    // int (*get_target_gid)(gid_t *);
        nullptr,    // unsigned int (*input_filter)(int arg, char *var, char **val, size_t val_len, size_t *new_val_len);
        nullptr,    // void (*ini_defaults)(HashTable *configuration_hash);
        1,          // phpinfo_as_text
        (char*)"html_errors=0\nregister_argc_argv=1\nimplicit_flush=1\noutput_buffering=0\nmax_execution_time=0\nmax_input_time=-1\n\0",
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
    return php_module_startup(sapi_module, g.exts.data(), g.exts.size());
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
    SAPIGlobals::instance().out->write(str, str_length).flush();
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

static void sapi_log_message(char* message)
{
    *(SAPIGlobals::instance().err) << message << std::endl;
}

}

TestSAPI::TestSAPI(std::ostream& out, std::ostream& err)
    : m_initialized(false)
{
#ifdef ZTS
    tsrm_startup(1, 1, 0, NULL);
    ts_resource(0);
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
#ifdef ZEND_SIGNALS
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
    g.exts.clear();

#ifdef ZTS
    tsrm_shutdown();
#endif
}

void TestSAPI::initialize()
{
    if (!this->m_initialized) {
        SAPIGlobals& g = SAPIGlobals::instance();
        g.sapi.startup(&g.sapi);
        this->m_initialized = true;
    }
}

void TestSAPI::addExtension(phpcxx::Extension& ext)
{
    if (!this->m_initialized) {
        SAPIGlobals& g = SAPIGlobals::instance();
        g.exts.push_back(*ext.module());
    }
}

void TestSAPI::run(std::function<void(void)> callback)
{
    this->initialize();

    SG(options) |= SAPI_OPTION_NO_CHDIR;
    php_request_startup();
    SG(headers_sent) = 1;
    SG(request_info).no_headers = 1;
    php_register_variable((char*)"PHP_SELF", (char*)"-", NULL);

    zend_first_try {
        callback();
    }
    zend_catch {
    }
    zend_end_try();

    if (EG(current_execute_data) && EG(current_execute_data)->opline) {
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

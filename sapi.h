#ifndef MY_SAPI_H
#define MY_SAPI_H

#include <main/php.h>
#include <main/php_main.h>
#include <main/SAPI.h>
#include <main/php_variables.h>
#include <Zend/zend_exceptions.h>
#include <iostream>

class MySAPI {
public:
    MySAPI()
    {
#ifdef ZTS
        tsrm_startup(1, 1, 0, NULL);
        ts_resource(0);
        ZEND_TSRMLS_CACHE_UPDATE();
#endif
#ifdef ZEND_SIGNALS
        zend_signal_startup();
#endif

        ::sapi_startup(&this->m_sapi);
        this->m_sapi.startup(&this->m_sapi);
    }

    void run(std::function<void(void)> callback)
    {
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

        while (EG(exception)) {
            zend_clear_exception();
        }

        php_request_shutdown(nullptr);
    }

    ~MySAPI()
    {
        php_module_shutdown();
        ::sapi_shutdown();
#ifdef ZTS
        tsrm_shutdown();
#endif
    }

    static int sapi_startup(sapi_module_struct *sapi_module)
    {
        return php_module_startup(sapi_module, nullptr, 0);
    }

    static int sapi_activate()
    {
        return SUCCESS;
    }

    static int sapi_deactivate()
    {
        std::cout << std::flush;
        return SUCCESS;
    }

    static size_t ub_write(const char* str, size_t str_length)
    {
        std::cout.write(str, str_length);
        return str_length;
    }

    static void flush(void*)
    {
        std::cout << std::flush;
    }

    static void send_header(sapi_header_struct*, void*)
    {
    }

    static char* read_cookies()
    {
        return nullptr;
    }

    static void log_message(char* message)
    {
        std::cerr << message << std::endl;
    }

private:
    zend_llist m_globals;
    sapi_module_struct m_sapi = {
        (char*)"mysapi",
        (char*)"My SAPI",
        &MySAPI::sapi_startup,
        php_module_shutdown_wrapper,
        &MySAPI::sapi_activate,
        &MySAPI::sapi_deactivate,
        &MySAPI::ub_write,
        &MySAPI::flush,
        nullptr,    // get UID
        nullptr,    // get env
        zend_error,
        nullptr,    // header handler
        nullptr,    // send headers handler
        &MySAPI::send_header,
        nullptr,    // read POST data
        &MySAPI::read_cookies,
        php_import_environment_variables,
        &MySAPI::log_message,
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

};

#endif /* SAPI_H_ */

#include <Zend/zend_API.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_objects.h>
#include "phpexception.h"
#include "phpexception_p.h"
#include "emallocallocator.h"

phpcxx::PhpExceptionPrivate::PhpExceptionPrivate(zend_object* obj)
    : m_previous(nullptr), m_handled(false)
{
    zval rv;
    zval ex;
    ZVAL_OBJ(&ex, obj);
    zend_class_entry* scope = Z_OBJCE(ex);
    zend_string* msg  = zval_get_string(zend_read_property(scope, &ex, ZEND_STRL("message"), 1, &rv));
    zend_string* file = zval_get_string(zend_read_property(scope, &ex, ZEND_STRL("file"), 1, &rv));
    zend_long   code  = zval_get_long(zend_read_property(scope, &ex, ZEND_STRL("code"), 1, &rv));
    zend_long   line  = zval_get_long(zend_read_property(scope, &ex, ZEND_STRL("line"), 1, &rv));

    this->m_class     = { ZSTR_VAL(scope->name), ZSTR_LEN(scope->name) };
    this->m_message   = { ZSTR_VAL(msg),         ZSTR_LEN(msg)         };
    this->m_file      = { ZSTR_VAL(file),        ZSTR_LEN(file)        };
    this->m_code      = code;
    this->m_line      = line;

    zend_string_release(msg);
    zend_string_release(file);

    zval* trace       = zend_read_property(scope, &ex, ZEND_STRL("trace"), 1, &rv);
    zval* previous    = zend_read_property(scope, &ex, ZEND_STRL("previous"), 1, &rv);

    if (trace && Z_TYPE_P(trace) == IS_ARRAY) {
        this->m_trace = trace;
    }

    if (previous && Z_TYPE_P(previous) == IS_OBJECT) {
        zend_object* p = Z_OBJ_P(previous);
        std::unique_ptr<PhpExceptionPrivate, emdeleter> ep(emcreate<PhpExceptionPrivate>(p), emdeleter());
        this->m_previous.reset(emcreate<PhpException>(ep.release()));
    }
}

phpcxx::PhpExceptionPrivate::~PhpExceptionPrivate()
{
    if (this->m_handled && !this->m_class.empty()) {
        zend_clear_exception();
    }
}

phpcxx::PhpException::PhpException()
    : d_ptr(emcreate<PhpExceptionPrivate>(EG(exception)), emdeleter())
{
}

phpcxx::PhpException::PhpException(phpcxx::PhpException&& other)
    : d_ptr(std::move(other.d_ptr))
{
}

const phpcxx::string& phpcxx::PhpException::getClass() const
{
    return this->d_ptr->getClass();
}

const phpcxx::string& phpcxx::PhpException::message() const
{
    return this->d_ptr->message();
}

const phpcxx::string& phpcxx::PhpException::file() const
{
    return this->d_ptr->file();
}

long int phpcxx::PhpException::code() const
{
    return this->d_ptr->code();
}

long int phpcxx::PhpException::line() const
{
    return this->d_ptr->line();
}

phpcxx::Value phpcxx::PhpException::trace() const
{
    return this->d_ptr->trace();
}

const phpcxx::PhpException* phpcxx::PhpException::previous() const
{
    return this->d_ptr->previous();
}

phpcxx::PhpException::PhpException(phpcxx::PhpExceptionPrivate* dd) noexcept
    : d_ptr(dd)
{
}

phpcxx::PhpException::~PhpException() noexcept
{
}

const char* phpcxx::PhpException::what() const noexcept
{
    return this->d_ptr->message().c_str();
}

void phpcxx::PhpException::markHandled(bool handled)
{
    this->d_ptr->markHandled(handled);
}

bool phpcxx::PhpException::isHandled() const
{
    return this->d_ptr->isHandled();
}

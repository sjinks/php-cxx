#ifndef PHPCXX_PHPEXCEPTION_P_H
#define PHPCXX_PHPEXCEPTION_P_H

#include "phpcxx.h"
#include "emallocallocator.h"
#include "array.h"

namespace phpcxx {

/**
 * @internal
 */
class PhpExceptionPrivate {
public:
    [[gnu::nonnull]] PhpExceptionPrivate(struct _zend_object* obj);
    ~PhpExceptionPrivate();

    const string& className() const { return this->m_class;   }
    const string& message() const   { return this->m_message; }
    const string& file() const      { return this->m_file;    }
    long int code() const           { return this->m_code;    }
    long int line() const           { return this->m_line;    }
    Array& trace() const            { return this->m_trace;   }
    const PhpException* previous() const { return this->m_previous.get(); }
    void passException()            { this->m_pass = true;    }

private:
    string m_class;
    string m_message;
    string m_file;
    long int m_code;
    long int m_line;
    std::unique_ptr<PhpException, emdeleter> m_previous;
    mutable Array m_trace;
    bool m_pass;
};

}

#endif /* PHPCXX_PHPEXCEPTION_P_H */

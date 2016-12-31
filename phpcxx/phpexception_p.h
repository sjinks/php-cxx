#ifndef PHPCXX_PHPEXCEPTION_P_H
#define PHPCXX_PHPEXCEPTION_P_H

#include "phpcxx.h"
#include "emallocallocator.h"
#include "value.h"

namespace phpcxx {

class PhpExceptionPrivate {
public:
    [[gnu::nonnull]] PhpExceptionPrivate(struct _zend_object* obj);
    ~PhpExceptionPrivate();

    const string& getClass() const { return this->m_class;   }
    const string& message() const  { return this->m_message; }
    const string& file() const     { return this->m_file;    }
    long int code() const          { return this->m_code;    }
    long int line() const          { return this->m_line;    }
    Value trace() const            { return this->m_trace;   }
    const PhpException* previous() const { return this->m_previous.get(); }

    void markHandled(bool handled) { this->m_handled = handled; }
    bool isHandled() const         { return this->m_handled;    }

private:
    string m_class;
    string m_message;
    string m_file;
    long int m_code;
    long int m_line;
    std::unique_ptr<PhpException, emdeleter> m_previous;
    Value m_trace;
    bool m_handled;
};

}

#endif /* PHPCXX_PHPEXCEPTION_P_H */

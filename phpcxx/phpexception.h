#ifndef PHPCXX_PHPEXCEPTION_H
#define PHPCXX_PHPEXCEPTION_H

#include "phpcxx.h"

#include <stdexcept>
#include <memory>
#include "emallocallocator.h"
#include "string.h"

struct _zend_object;

namespace phpcxx {

class Value;

class PhpExceptionPrivate;
class PHPCXX_EXPORT PhpException : public std::exception {
public:
    PhpException();
    PhpException(PhpExceptionPrivate* dd) noexcept;
    PhpException(const PhpException&) = delete;
    PhpException(PhpException&&);
    virtual ~PhpException() noexcept override;

    virtual const char* what() const noexcept override;

    const string& getClass() const;
    const string& message() const;
    const string& file() const;
    long int code() const;
    long int line() const;
    Value trace() const;
    const PhpException* previous() const;

    void markHandled(bool handled);
    bool isHandled() const;

private:
    friend class PhpExceptionPrivate;
    std::unique_ptr<PhpExceptionPrivate, emdeleter> d_ptr;
};

}

#endif /* PHPCXX_PHPEXCEPTION_H */

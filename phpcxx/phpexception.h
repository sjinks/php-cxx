#ifndef PHPCXX_PHPEXCEPTION_H
#define PHPCXX_PHPEXCEPTION_H

#include "phpcxx.h"

#include <stdexcept>
#include <memory>
#include "emallocallocator.h"
#include "string.h"

namespace phpcxx {

class Array;
class PhpExceptionPrivate;

/**
 * @brief Represents an exception thrown from PHP code
 */
class PHPCXX_EXPORT PhpException : public std::exception {
public:
    /**
     * @brief Default constructor
     * @throws std::logic_error `EG(exception) == nullptr`
     *
     * Constructs `PhpException` from `EG(exception)`
     */
    PhpException();

    /**
     * @internal
     */
    [[gnu::nonnull]] PhpException(PhpExceptionPrivate* dd) noexcept;

    /**
     * @brief Move constructor
     * @param other Exception being moved
     * @warning Calling any method of `other` will result in a crash
     * @warning This constructor is not intended to be called by the user
     */
    PhpException(PhpException&& other);

    /**
     * @brief Destructor
     */
    virtual ~PhpException() noexcept override;

    /**
     * @brief Returns the explanatory string about the exception
     */
    virtual const char* what() const noexcept override;

    /**
     * @brief Returns the name of the thrown class (ie, `Exception`)
     * @return Name of the class
     */
    const string& className() const;

    /**
     * @brief Returns the exception message
     * @return Exception message
     */
    const string& message() const;

    /**
     * @brief Returns the name of the file which threw the exception
     * @return Name of the file
     */
    const string& file() const;

    /**
     * @brief Returns exception code
     * @return Exception code
     */
    long int code() const;

    /**
     * @brief Returns the line number where the exception was thrown
     * @return Line number
     */
    long int line() const;

    /**
     * @brief Returns the backtrace
     * @return Backtrace
     */
    Array& trace() const;

    /**
     * @brief Returns previous exception
     * @return Previous exception
     * @retval `nullptr` There was no previous exception
     */
    const PhpException* previous() const;

private:
    friend class PhpExceptionPrivate;

    /**
     * @brief Implementation details
     */
    std::unique_ptr<PhpExceptionPrivate, emdeleter> d_ptr;

    friend class FunctionHandler;

    /**
     * @internal
     * @brief Sets the flag which tells the destructor not to clear PHP exception
     */
    void passException();
};

}

#endif /* PHPCXX_PHPEXCEPTION_H */

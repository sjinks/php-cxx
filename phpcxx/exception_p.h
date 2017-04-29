#ifndef PHPCXX_EXCEPTION_P_H
#define PHPCXX_EXCEPTION_P_H

#include "phpcxx.h"

/**
 * @internal
 * @brief Registers PhpCxx\\Exception PHP class
 * @return Whether registration succeeded
 * @retval SUCCESS
 * @retval FAILURE
 */
PHPCXX_HIDDEN int setup_phpcxx_exception();

#endif /* PHPCXX_EXCEPTION_P_H */

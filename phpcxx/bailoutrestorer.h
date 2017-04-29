#ifndef PHPCXX_BAILOUTRESTORER_H
#define PHPCXX_BAILOUTRESTORER_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_API.h>
}

namespace phpcxx {

/**
 * @brief Restores the value of `EG(bailout)` on destruction
 */
class PHPCXX_EXPORT BailoutRestorer {
public:
    /**
     * @brief Default constructor
     * @details Saves the value of `EG(bailout)`
     */
    BailoutRestorer() : m_orig_bailout(EG(bailout)) {}

    /**
     * @brief Destructor
     * @details Restores the saved value of `EG(bailout)`
     */
    ~BailoutRestorer()
    {
        EG(bailout) = this->m_orig_bailout;
    }

private:
    /**
     * @brief The original value of `EG(bailout)`
     */
    JMP_BUF* m_orig_bailout;
};

}

#endif /* PHPCXX_BAILOUTRESTORER_H */

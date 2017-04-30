#ifndef TEST_GLOBALS_H
#define TEST_GLOBALS_H

#include <cstring>

#include "phpcxx/string.h"

extern "C" {
#include <Zend/zend.h>
}

class DummyStackFrame {
public:
    DummyStackFrame()
    {
        if (!EG(current_execute_data)) {
            std::memset(&this->m_data, 0, sizeof(this->m_data));
            EG(current_execute_data) = &this->m_data;
        }
    }

    DummyStackFrame(const DummyStackFrame&) = delete;

    ~DummyStackFrame()
    {
        if (EG(current_execute_data) == &this->m_data) {
            EG(current_execute_data) = this->m_data.prev_execute_data;
        }
    }

private:
    zend_execute_data m_data;
};

void runPhpCode(const phpcxx::string& code);
void runPhpCode(const phpcxx::string& code, zval& retval);

#endif /* TEST_GLOBALS_H */

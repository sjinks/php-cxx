#include <cstdlib>
#include <iostream>
#include <string>
#include <gtest/gtest.h>

extern "C" {
#include <Zend/zend.h>
}

#include "valgrind/valgrind.h"
#include "globals.h"
#include "testsapi.h"

void runPhpCode(const phpcxx::string& code)
{
    char* desc = zend_make_compiled_string_description("eval'd code");
    zend_eval_stringl_ex(const_cast<char*>(code.c_str()), code.size(), nullptr, desc, 1);
    efree(desc);
}

void runPhpCode(const phpcxx::string& code, zval& retval)
{
    char* desc = zend_make_compiled_string_description("eval'd code");
    zend_eval_stringl_ex(const_cast<char*>(code.c_str()), code.size(), &retval, desc, 1);
    efree(desc);
}

int main(int argc, char** argv)
{
    if (RUNNING_ON_VALGRIND) {
        std::cout << "Valgrind detected";
        if (!std::getenv("USE_ZEND_ALLOC")) {
            std::cout << ", setting USE_ZEND_ALLOC to 0";
            setenv("USE_ZEND_ALLOC", "0", 0);
        }

        std::cout << std::endl;
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

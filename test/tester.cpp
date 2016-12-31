#include <iostream>
#include <string>
#include <gtest/gtest.h>
#include <Zend/zend.h>

#include "globals.h"
#include "testsapi.h"

void runPhpCode(const phpcxx::string& code)
{
    char* desc = zend_make_compiled_string_description("eval'd code");
    zend_eval_stringl_ex(const_cast<char*>(code.c_str()), code.size(), nullptr, desc, 1);
    efree(desc);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

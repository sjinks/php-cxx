#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "phpcxx/module.h"
#include "phpcxx/bailoutrestorer.h"
#include "testsapi.h"

TEST(BailoutRestorer, TestBailoutRestorer)
{
    phpcxx::Module module("BailoutRestorer", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        JMP_BUF* b = EG(bailout);
        {
            phpcxx::BailoutRestorer br;
        }
        EXPECT_EQ(b, EG(bailout));

        {
            phpcxx::BailoutRestorer br;
            EG(bailout) = nullptr;
        }
        EXPECT_EQ(b, EG(bailout));

        EG(bailout) = b;

        // Issue #4: if we don't run php_request_startup() and php_request_shutdown() after sapi.initialize()
        // we can get memory corruption in NTS builds
        sapi.run();
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);
}

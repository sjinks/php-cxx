#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "phpcxx/callable.h"
#include "phpcxx/superglobal.h"
#include "testsapi.h"

extern "C" {
#include <Zend/zend.h>
}

TEST(Callable, TestConstruct)
{
    phpcxx::Module module("Callable", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        sapi.run([] {
            runPhpCode(R"___(
class TestClass
{
    public static function staticMethod()
    {
    }

    public function normalMethod()
    {
    }
}

$a = new TestClass;
)___");

            phpcxx::SuperGlobal GLOBALS("GLOBALS");
            phpcxx::Value a = GLOBALS["a"];
            phpcxx::Array b;
            b[nullptr] = "testclass";
            b[nullptr] = "staticmethod";

            phpcxx::Callable c1("sort");
            phpcxx::Callable c2("TestClass", "staticMethod");
            EXPECT_NO_THROW(phpcxx::Callable c3(a.pzval(), "normalMethod"));
            EXPECT_NO_THROW(phpcxx::Callable c4(b.pzval()));

            phpcxx::Value v(1);
            EXPECT_THROW(phpcxx::Callable(v.pzval()), std::invalid_argument);
            EXPECT_THROW(phpcxx::Callable(v.pzval(), "method"), std::invalid_argument);

            b[0] = v;
            EXPECT_THROW(phpcxx::Callable(b.pzval()), std::invalid_argument);
        });
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);
}

TEST(Callable, TestResolve)
{
    phpcxx::Module module("Callable", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        sapi.run([] {
            zend_fcall_info fci;
            zend_fcall_info_cache fcc;
            zend_fcall_info_cache fcc2;
            phpcxx::Callable c1("sort");

            EXPECT_TRUE(c1.resolve(fci, fcc));
            EXPECT_EQ(sizeof(fci), fci.size);
            EXPECT_EQ(1, fcc.initialized);
            EXPECT_NE(nullptr, fcc.function_handler);
            fcc2 = fcc;

            phpcxx::Callable c2("this_function_does_not_exist");
            EXPECT_FALSE(c2.resolve(fci, fcc));
            EXPECT_EQ(sizeof(fci), fci.size);
            EXPECT_EQ(0, fcc.initialized);
            EXPECT_EQ(nullptr, fcc.function_handler);

            phpcxx::Callable c3(std::move(c1));
            EXPECT_TRUE(c3.resolve(fci, fcc));
            EXPECT_EQ(sizeof(fci), fci.size);
            EXPECT_EQ(1, fcc.initialized);
            EXPECT_EQ(fcc2.function_handler, fcc.function_handler);

            EXPECT_FALSE(c1.resolve(fci, fcc));
            EXPECT_EQ(0, fcc.initialized);
            EXPECT_EQ(nullptr, fcc.function_handler);
        });
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);
}

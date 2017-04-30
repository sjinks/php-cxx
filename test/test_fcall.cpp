#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "phpcxx/module.h"
#include "phpcxx/function.h"
#include "phpcxx/fcall.h"
#include "phpcxx/types.h"
#include "phpcxx/vector.h"
#include "testsapi.h"
#include "globals.h"

extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_exceptions.h>
}

namespace {

class MyModule : public phpcxx::Module {
public:
    using phpcxx::Module::Module;

protected:
    virtual std::vector<phpcxx::Function> functions() override
    {
        return {
            phpcxx::createFunction<&MyModule::func_bailingout>("func_bailingout")
        };
    }

private:
    static void func_bailingout()
    {
        php_printf("+ %s\n", "func_bailingout");
        zend_error(E_ERROR, "Bailing out");
    }
};

}

TEST(FCall, TestExceptions)
{
    phpcxx::Module module("FCall", "0.0");

    std::stringstream out;
    std::stringstream err;
    std::string o;
    std::string e;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        sapi.run([]() {
            zval undef;
            ZVAL_UNDEF(&undef);

            EXPECT_THROW(phpcxx::FCall("non_existing_function"), std::invalid_argument);
            EXPECT_THROW(phpcxx::FCall("stdClass", "non_existing_function"), std::invalid_argument);
            EXPECT_THROW(phpcxx::FCall(&undef, "non_existing_function"), std::invalid_argument);

            zend_fcall_info fci;
            zend_fcall_info_cache fcc;
            fci.size = 0;

            EXPECT_THROW(phpcxx::FCall(fci, fcc), std::invalid_argument);

            phpcxx::Value v("non_existing_function");
            fcc.initialized = 0;

            zval retval;
            ZVAL_NULL(&retval);
            ZVAL_COPY_VALUE(&fci.function_name, v.pzval());
            fci.size           = sizeof(fci);
            fci.object         = nullptr;
            fci.param_count    = 0;
            fci.params         = nullptr;
            fci.no_separation  = 1;
            fci.retval         = &retval;
#if PHP_VERSION_ID < 70100
            fci.function_table = EG(function_table);
            fci.symbol_table   = nullptr;
#endif
            phpcxx::FCall c(fci, fcc);
            EXPECT_THROW(c(phpcxx::vector<phpcxx::Value>{v}), std::runtime_error);
            EXPECT_THROW(c(phpcxx::vector<zval>{retval}), std::runtime_error);
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("", o);
        EXPECT_EQ("", e);
    }
}

TEST(FCall, TestBailout)
{
    MyModule module("FCall", "0.0");

    std::stringstream out;
    std::stringstream err;
    std::string o;
    std::string e;
    TestSAPI sapi(out, err);
    sapi.addModule(module);
    sapi.initialize();

    {
        bool bailed_out = false;

        sapi.run([&bailed_out]() {
            DummyStackFrame dummy;
            zend_try {
                phpcxx::call("func_bailingout");
                ADD_FAILURE();
            }
            zend_catch {
                bailed_out = true;
            }
            zend_end_try()

            EXPECT_TRUE(bailed_out);
        });

        EXPECT_TRUE(bailed_out);

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("+ func_bailingout\n", o);
        EXPECT_NE(std::string::npos, e.find("Bailing out"));
        EXPECT_NE(std::string::npos, e.find("Fatal error"));
    }

    {
        bool bailed_out = false;

        sapi.run([&bailed_out]() {
            DummyStackFrame dummy;
            zend_try {
                phpcxx::Value f("func_bailingout");
                phpcxx::call(f);
                ADD_FAILURE();
            }
            zend_catch {
                bailed_out = true;
            }
            zend_end_try()

            EXPECT_TRUE(bailed_out);
        });

        EXPECT_TRUE(bailed_out);

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("+ func_bailingout\n", o);
        EXPECT_NE(std::string::npos, e.find("Bailing out"));
        EXPECT_NE(std::string::npos, e.find("Fatal error"));
    }
}

TEST(FCall, TestNormalCall)
{
    MyModule module("FCall", "0.0");

    std::stringstream out;
    std::stringstream err;
    std::string o;
    std::string e;
    TestSAPI sapi(out, err);
    sapi.addModule(module);
    sapi.initialize();

    {
        bool bailed_out = false;

        sapi.run([&bailed_out]() {
            DummyStackFrame dummy;

            runPhpCode(R"___(
class Test
{
    public static function f()
    {
        return true;
    }
}
)___");

            phpcxx::Value v;

            phpcxx::FCall c1("Test::f");
            v = c1();
            EXPECT_EQ(phpcxx::Type::True, v.type());
            v = nullptr;

            phpcxx::FCall c2("Test", "f");
            v = c2();
            EXPECT_EQ(phpcxx::Type::True, v.type());
            v = nullptr;

            phpcxx::Value t("Test");
            phpcxx::FCall c3(t.pzval(), "f");
            v = c3();
            EXPECT_EQ(phpcxx::Type::True, v.type());
            v = nullptr;
        });
    }
}

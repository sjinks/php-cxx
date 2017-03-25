#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <Zend/zend.h>
#include <Zend/zend_constants.h>
#include "phpcxx/module.h"
#include "phpcxx/constant.h"
#include "testsapi.h"

namespace {

class MyModule : public phpcxx::Module {
public:
    using phpcxx::Module::Module;

protected:
    virtual std::vector<phpcxx::Constant> constants()
    {
        phpcxx::Constant bool_const("BOOL_CONST", true);
        phpcxx::Constant int_const("INT_CONST", 10);
        phpcxx::Constant long_const("LONG_CONST", static_cast<zend_long>(100));
        phpcxx::Constant double_const("DOUBLE_CONST", 200.0);
        phpcxx::Constant float_const("FLOAT_CONST", 300.0f);
        phpcxx::Constant null_const("NULL_CONST");
        phpcxx::Constant string_const("STRING_CONST", "string");
        phpcxx::Constant string_const2("STRING_CONST2", std::string("std::string"));

        phpcxx::Constant not_used("NOT_USED_CONSTANT", "if a constant is not registered, it should not cause a memory leak");
        phpcxx::Constant duplicate("STRING_CONST", "a duplicate constant should not cause memory leak");

        return { bool_const, int_const, long_const, double_const, float_const, null_const, string_const, string_const2, duplicate };
    }
};

}

TEST(ConstantsTest, TestConstants)
{
    MyModule module("Constants", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        std::string o = out.str(); out.str("");
        std::string e = err.str(); err.str("");
        EXPECT_NE(std::string::npos, e.find("Constant STRING_CONST already defined"));
        EXPECT_EQ("", o);

        zval* bc  = zend_get_constant_str(ZEND_STRL("BOOL_CONST"));
        zval* ic  = zend_get_constant_str(ZEND_STRL("INT_CONST"));
        zval* lc  = zend_get_constant_str(ZEND_STRL("LONG_CONST"));
        zval* dc  = zend_get_constant_str(ZEND_STRL("DOUBLE_CONST"));
        zval* fc  = zend_get_constant_str(ZEND_STRL("FLOAT_CONST"));
        zval* nc  = zend_get_constant_str(ZEND_STRL("NULL_CONST"));
        zval* sc1 = zend_get_constant_str(ZEND_STRL("STRING_CONST"));
        zval* sc2 = zend_get_constant_str(ZEND_STRL("STRING_CONST2"));
        zval* uc  = zend_get_constant_str(ZEND_STRL("NOT_USED_CONSTANT"));

        ASSERT_TRUE(bc  != nullptr);
        ASSERT_TRUE(ic  != nullptr);
        ASSERT_TRUE(lc  != nullptr);
        ASSERT_TRUE(dc  != nullptr);
        ASSERT_TRUE(fc  != nullptr);
        ASSERT_TRUE(nc  != nullptr);
        ASSERT_TRUE(sc1 != nullptr);
        ASSERT_TRUE(sc2  != nullptr);
        EXPECT_EQ(uc, nullptr);

        EXPECT_EQ(IS_TRUE,   Z_TYPE_P(bc));

        EXPECT_EQ(IS_LONG,   Z_TYPE_P(ic));
        EXPECT_EQ(IS_LONG,   Z_TYPE_P(lc));
        EXPECT_EQ(10,        Z_LVAL_P(ic));
        EXPECT_EQ(100,       Z_LVAL_P(lc));

        EXPECT_EQ(IS_DOUBLE, Z_TYPE_P(dc));
        EXPECT_EQ(IS_DOUBLE, Z_TYPE_P(fc));
        EXPECT_EQ(200,       Z_DVAL_P(dc));
        EXPECT_EQ(300,       Z_DVAL_P(fc));

        EXPECT_EQ(IS_NULL,   Z_TYPE_P(nc));

        ASSERT_EQ(IS_STRING, Z_TYPE_P(sc1));
        ASSERT_EQ(IS_STRING, Z_TYPE_P(sc2));

        EXPECT_STREQ(Z_STRVAL_P(sc1), "string");
        EXPECT_STREQ(Z_STRVAL_P(sc2), "std::string");

        // Issue #4: if we don't run php_request_startup() and php_request_shutdown() after sapi.initialize()
        // we can get memory corruption in NTS builds
        sapi.run();
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);
}

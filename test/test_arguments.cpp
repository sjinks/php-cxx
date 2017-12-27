#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "phpcxx/argument.h"
#include "testsapi.h"

extern "C" {
#include <Zend/zend.h>
}

//#if PHP_VERSION_ID < 70200
TEST(ArgumentsTest, TestNative)
{
    static const zend_internal_arg_info args[8] = {
        ZEND_ARG_INFO(0, byval)
        ZEND_ARG_INFO(1, byref)
        ZEND_ARG_ARRAY_INFO(1, nullable_array, 1)
        ZEND_ARG_OBJ_INFO(0, stdclass, stdClass, 0)
        ZEND_ARG_CALLABLE_INFO(0, callable, 0)
        ZEND_ARG_TYPE_INFO(0, boolean, _IS_BOOL, 0)
        ZEND_ARG_VARIADIC_INFO(1, variadic_byref)
        { }
    };

    std::stringstream out;
    std::stringstream err;
    TestSAPI sapi(out, err);

    sapi.run([]() {
        std::stringstream stream;
        std::string s;

        {
            phpcxx::Argument a(args[0]);
            EXPECT_STREQ("byval", a.name());
            EXPECT_FALSE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Any, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=byval, type=undefined, nullable=0, byref=0, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a(args[1]);
            EXPECT_STREQ("byref", a.name());
            EXPECT_TRUE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Any, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=byref, type=undefined, nullable=0, byref=1, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a(args[2]);
            EXPECT_STREQ("nullable_array", a.name());
            EXPECT_TRUE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Array, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_TRUE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=nullable_array, type=array, nullable=1, byref=1, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a(args[3]);
            EXPECT_STREQ("stdclass", a.name());
            EXPECT_FALSE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Object, a.type());
            EXPECT_STREQ("stdClass", a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=stdclass, type=object, nullable=0, byref=0, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a(args[4]);
            EXPECT_STREQ("callable", a.name());
            EXPECT_FALSE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Callable, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=callable, type=callable, nullable=0, byref=0, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a(args[5]);
            EXPECT_STREQ("boolean", a.name());
            EXPECT_FALSE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Bool, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=boolean, type=boolean, nullable=0, byref=0, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a(args[6]);
            EXPECT_STREQ("variadic_byref", a.name());
            EXPECT_TRUE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Any, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_TRUE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=variadic_byref, type=undefined, nullable=0, byref=1, variadic=1]\n", s);
        }
    });
}
//#endif

TEST(ArgumentsTest, TestPhpCxx)
{
    std::stringstream out;
    std::stringstream err;
    TestSAPI sapi(out, err);

    sapi.run([]() {
        std::stringstream stream;
        std::string s;

        {
            phpcxx::Argument a("byval");

            EXPECT_STREQ("byval", a.name());
            EXPECT_FALSE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Any, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=byval, type=undefined, nullable=0, byref=0, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a("byref");
            a.setByRef(true);

            EXPECT_STREQ("byref", a.name());
            EXPECT_TRUE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Any, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=byref, type=undefined, nullable=0, byref=1, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a("nullable_array");
            a.setByRef(true).setType(phpcxx::ArgumentType::Array, true);

            EXPECT_STREQ("nullable_array", a.name());
            EXPECT_TRUE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Array, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_TRUE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=nullable_array, type=array, nullable=1, byref=1, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a("stdclass");
            a.setByRef(false).setClass("stdClass", false);

            EXPECT_STREQ("stdclass", a.name());
            EXPECT_FALSE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Object, a.type());
            EXPECT_STREQ("stdClass", a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=stdclass, type=object, nullable=0, byref=0, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a("callable");
            a.setType(phpcxx::ArgumentType::Callable, false);

            EXPECT_STREQ("callable", a.name());
            EXPECT_FALSE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Callable, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=callable, type=callable, nullable=0, byref=0, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a("boolean");
            a.setType(phpcxx::ArgumentType::Bool, false);

            EXPECT_STREQ("boolean", a.name());
            EXPECT_FALSE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Bool, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_FALSE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=boolean, type=boolean, nullable=0, byref=0, variadic=0]\n", s);
        }

        {
            phpcxx::Argument a("variadic_byref");
            a.setByRef(true);
            a.setVariadic(true);

            EXPECT_STREQ("variadic_byref", a.name());
            EXPECT_TRUE(a.isPassedByReference());
            EXPECT_EQ(phpcxx::ArgumentType::Any, a.type());
            EXPECT_EQ(nullptr, a.className());
            EXPECT_FALSE(a.canBeNull());
            EXPECT_TRUE(a.isVariadic());

            stream << a;
            s = stream.str(); stream.str("");
            EXPECT_EQ("[Argument: name=variadic_byref, type=undefined, nullable=0, byref=1, variadic=1]\n", s);
        }
    });
}

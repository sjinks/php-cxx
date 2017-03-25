#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <Zend/zend.h>
#include "phpcxx/argument.h"

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

    {
        phpcxx::Argument a(args[0]);
        EXPECT_STREQ("byval", a.name());
        EXPECT_FALSE(a.isPassedByReference());
        EXPECT_EQ(phpcxx::ArgumentType::Any, a.type());
        EXPECT_EQ(nullptr, a.className());
        EXPECT_FALSE(a.canBeNull());
        EXPECT_FALSE(a.isVariadic());
    }

    {
        phpcxx::Argument a(args[1]);
        EXPECT_STREQ("byref", a.name());
        EXPECT_TRUE(a.isPassedByReference());
        EXPECT_EQ(phpcxx::ArgumentType::Any, a.type());
        EXPECT_EQ(nullptr, a.className());
        EXPECT_FALSE(a.canBeNull());
        EXPECT_FALSE(a.isVariadic());
    }

    {
        phpcxx::Argument a(args[2]);
        EXPECT_STREQ("nullable_array", a.name());
        EXPECT_TRUE(a.isPassedByReference());
        EXPECT_EQ(phpcxx::ArgumentType::Array, a.type());
        EXPECT_EQ(nullptr, a.className());
        EXPECT_TRUE(a.canBeNull());
        EXPECT_FALSE(a.isVariadic());
    }

    {
        phpcxx::Argument a(args[3]);
        EXPECT_STREQ("stdclass", a.name());
        EXPECT_FALSE(a.isPassedByReference());
        EXPECT_EQ(phpcxx::ArgumentType::Object, a.type());
        EXPECT_STREQ("stdClass", a.className());
        EXPECT_FALSE(a.canBeNull());
        EXPECT_FALSE(a.isVariadic());
    }

    {
        phpcxx::Argument a(args[4]);
        EXPECT_STREQ("callable", a.name());
        EXPECT_FALSE(a.isPassedByReference());
        EXPECT_EQ(phpcxx::ArgumentType::Callable, a.type());
        EXPECT_EQ(nullptr, a.className());
        EXPECT_FALSE(a.canBeNull());
        EXPECT_FALSE(a.isVariadic());
    }

    {
        phpcxx::Argument a(args[5]);
        EXPECT_STREQ("boolean", a.name());
        EXPECT_FALSE(a.isPassedByReference());
        EXPECT_EQ(phpcxx::ArgumentType::Bool, a.type());
        EXPECT_EQ(nullptr, a.className());
        EXPECT_FALSE(a.canBeNull());
        EXPECT_FALSE(a.isVariadic());
    }

    {
        phpcxx::Argument a(args[6]);
        EXPECT_STREQ("variadic_byref", a.name());
        EXPECT_TRUE(a.isPassedByReference());
        EXPECT_EQ(phpcxx::ArgumentType::Any, a.type());
        EXPECT_EQ(nullptr, a.className());
        EXPECT_FALSE(a.canBeNull());
        EXPECT_TRUE(a.isVariadic());
    }
}

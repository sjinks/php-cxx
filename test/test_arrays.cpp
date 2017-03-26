#include <sstream>
#include <gtest/gtest.h>
#include "phpcxx/array.h"
#include "phpcxx/operators.h"
#include "phpcxx/value.h"
#include "globals.h"
#include "testsapi.h"

namespace {

class CommonFixture : public ::testing::Test {
protected:
    std::stringstream m_out;
    std::stringstream m_err;
    TestSAPI m_sapi;

    CommonFixture()
        : m_out(), m_err(), m_sapi(m_out, m_err)
    {
        m_sapi.setOutputStream(&m_out);
        m_sapi.setErrorStream(&m_err);
    }
};

typedef CommonFixture ArrayFixture;
}

TEST_F(ArrayFixture, InitializationNormal)
{
    m_sapi.run([this]() {
        phpcxx::Value t;
        t["a"] = 1;

        phpcxx::Array a1;
        phpcxx::Array a2(t);
        phpcxx::Array a3(a2);

        zval* z1 = a1.pzval();
        zval* z2 = a2.pzval();
        zval* z3 = a3.pzval();

        ASSERT_TRUE(z1 != nullptr);
        ASSERT_TRUE(z2 != nullptr);
        ASSERT_TRUE(z3 != nullptr);
        EXPECT_EQ(IS_ARRAY, Z_TYPE_P(z1));
        EXPECT_EQ(IS_ARRAY, Z_TYPE_P(z2));
        EXPECT_EQ(IS_ARRAY, Z_TYPE_P(z3));
        EXPECT_EQ(1, Z_REFCOUNT_P(z1));
        EXPECT_EQ(3, Z_REFCOUNT_P(z2));
        EXPECT_EQ(3, Z_REFCOUNT_P(z3));
        EXPECT_EQ(Z_ARRVAL_P(z2), Z_ARRVAL_P(z3));
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());
}

TEST_F(ArrayFixture, InitializationReferences)
{
    m_sapi.run([this]() {
        phpcxx::Value t1;
        t1["a"] = 1;

        phpcxx::Value t2 = t1.reference();

        phpcxx::Array a1(t1);
        zval* z1 = a1.pzval();

        ASSERT_TRUE(z1 != nullptr);
        EXPECT_EQ(IS_ARRAY, Z_TYPE_P(z1));
        EXPECT_EQ(2, Z_REFCOUNT_P(z1));

        phpcxx::Array a2(t2);
        zval* z2 = a2.pzval();

        ASSERT_TRUE(z2 != nullptr);
        EXPECT_EQ(IS_ARRAY, Z_TYPE_P(z2));

        EXPECT_EQ(3, Z_REFCOUNT_P(z1));
        EXPECT_EQ(3, Z_REFCOUNT_P(z2));
        EXPECT_EQ(Z_ARRVAL_P(z1), Z_ARRVAL_P(z2));

        EXPECT_EQ(2, t1.refCount());
        EXPECT_EQ(2, t2.refCount());
        EXPECT_TRUE(t1.isReference());
        EXPECT_TRUE(t2.isReference());

        phpcxx::Value& r1 = t1.dereference();
        phpcxx::Value& r2 = t2.dereference();
        EXPECT_EQ(3, r1.refCount());
        EXPECT_EQ(3, r2.refCount());

        EXPECT_EQ(phpcxx::Type::Array, r1.type());
        EXPECT_EQ(phpcxx::Type::Array, r2.type());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());
}

TEST_F(ArrayFixture, InitializationConversion)
{
    m_sapi.run([this]() {
        phpcxx::Value t1(4);
        phpcxx::Value t2(nullptr);

        phpcxx::Array a1(t1);
        phpcxx::Array a2(t2);
        zval* z1 = a1.pzval();
        zval* z2 = a2.pzval();

        ASSERT_TRUE(z1 != nullptr);
        ASSERT_TRUE(z2 != nullptr);
        EXPECT_EQ(IS_ARRAY, Z_TYPE_P(z1));
        EXPECT_EQ(IS_ARRAY, Z_TYPE_P(z2));

        EXPECT_EQ(phpcxx::Type::Integer, t1.type());
        EXPECT_EQ(phpcxx::Type::Null,    t2.type());
        EXPECT_EQ(4,                     t1.asLong());

        EXPECT_EQ(1, a1.size());
        EXPECT_EQ(0, a2.size());

        EXPECT_TRUE(a1.contains("0"));
        EXPECT_TRUE(a1.contains(static_cast<zend_long>(0)));
        EXPECT_FALSE(a2.contains(static_cast<zend_long>(0)));

        EXPECT_EQ(a1["0"], a1[static_cast<zend_long>(0)]);
        EXPECT_EQ(4, a1[static_cast<zend_long>(0)].asLong());

        EXPECT_EQ(phpcxx::Type::Null, a1[1].type());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());
}

TEST_F(ArrayFixture, Arrays)
{
    m_sapi.run([this]() {
        phpcxx::Value a;
        a["a"]["b"]["c"] = 1;
        a["a"]["b"]["d"] = 2;
        a["a"]["e"]      = 3;
        phpcxx::Value func("var_export");
        func(a);

        std::string expected = "array (\n  'a' => \n  array (\n    'b' => \n    array (\n      'c' => 1,\n      'd' => 2,\n    ),\n    'e' => 3,\n  ),\n)";
        std::string actual   = m_out.str();
        m_out.str(std::string());
        EXPECT_EQ(expected, actual);
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value a;
        phpcxx::Value b;
        a["a"] = 1;
        b = a["a"];

        EXPECT_EQ(phpcxx::Type::Integer, b.type());
        EXPECT_EQ(1, b.asLong());

        a["a"] = 2;

        EXPECT_EQ(phpcxx::Type::Integer, b.type());
        EXPECT_EQ(1, b.asLong());
        EXPECT_EQ(2, a["a"].asLong());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::ZendString GLOBALS("GLOBALS");
        phpcxx::ZendString SERVER("_SERVER");
        zend_is_auto_global(GLOBALS.get());
        zval* g = zend_hash_find_ind(&EG(symbol_table), GLOBALS.get());
        ASSERT_TRUE(g != nullptr);
        EXPECT_TRUE(Z_TYPE_P(g) == IS_ARRAY || (Z_TYPE_P(g) == IS_REFERENCE && Z_TYPE_P(Z_REFVAL_P(g)) == IS_ARRAY));

        phpcxx::Value globals(g);

        runPhpCode("global $a; $a = 14;");
        phpcxx::Value& a = globals["a"];
        EXPECT_EQ(14, a.asLong());

        zend_is_auto_global(SERVER.get());
        phpcxx::Value server(&PG(http_globals)[TRACK_VARS_SERVER]);
        EXPECT_EQ(phpcxx::Type::Array, server.type());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    /**/
    m_sapi.run([this]() {
        phpcxx::Array a;
        a["a"]["b"]["c"] = 1;
        a["a"]["b"]["d"] = 2;
        a["a"]["e"]      = 3;
        phpcxx::Value func("var_export");
        func(a);

        EXPECT_TRUE(a.contains("a"));
        EXPECT_FALSE(a.contains("b"));
        EXPECT_EQ(1, a.size());

        phpcxx::Array aa(a["a"]);
        phpcxx::Value e("e");
        EXPECT_TRUE(aa.contains(phpcxx::string("b")));
        EXPECT_TRUE(aa.contains(e));
        EXPECT_EQ(2, aa.size());

        a["f"] = "smth";
        EXPECT_TRUE(a.contains("f"));
        EXPECT_EQ(2, a.size());
        a.unset("f");
        EXPECT_FALSE(a.contains("f"));
        EXPECT_EQ(1, a.size());

        std::string expected = "array (\n  'a' => \n  array (\n    'b' => \n    array (\n      'c' => 1,\n      'd' => 2,\n    ),\n    'e' => 3,\n  ),\n)";
        std::string actual   = m_out.str();
        m_out.str(std::string());

        EXPECT_EQ(expected, actual);
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Array a;
        phpcxx::Value b;
        a["a"] = 1;
        b = a["a"];

        EXPECT_EQ(phpcxx::Type::Integer, b.type());
        EXPECT_EQ(1, b.asLong());

        a["a"] = 2;

        EXPECT_EQ(phpcxx::Type::Integer, b.type());
        EXPECT_EQ(1, b.asLong());
        EXPECT_EQ(2, a["a"].asLong());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::ZendString GLOBALS("GLOBALS");
        phpcxx::ZendString SERVER("_SERVER");
        zend_is_auto_global(GLOBALS.get());
        zval* g = zend_hash_find_ind(&EG(symbol_table), GLOBALS.get());
        ASSERT_TRUE(g != nullptr);
        EXPECT_TRUE(Z_TYPE_P(g) == IS_ARRAY || (Z_TYPE_P(g) == IS_REFERENCE && Z_TYPE_P(Z_REFVAL_P(g)) == IS_ARRAY));

        phpcxx::Array globals(g);

        runPhpCode("global $a; $a = 14;");
        phpcxx::Value& a = globals["a"];
        EXPECT_EQ(14, a.asLong());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());
}

#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "phpcxx/array.h"
#include "phpcxx/operators.h"
#include "phpcxx/value.h"
#include "phpcxx/string.h"
#include "phpcxx/map.h"
#include "phpcxx/vector.h"
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

        EXPECT_EQ(0, a1.size());
        EXPECT_EQ(1, a2.size());
        EXPECT_EQ(1, a3.size());

        phpcxx::Array a4(std::move(a3));
        z3       = a3.pzval();
        zval* z4 = a4.pzval();

        EXPECT_EQ(IS_UNDEF, Z_TYPE_P(z3));
        EXPECT_EQ(IS_ARRAY, Z_TYPE_P(z4));
        EXPECT_EQ(1, a4.size());
    });
    EXPECT_EQ("", m_err.str());
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
    EXPECT_EQ("", m_err.str());
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

        EXPECT_TRUE(a1.isset("0"));
        EXPECT_TRUE(a1.isset(0));
        EXPECT_FALSE(a2.isset(static_cast<zend_long>(0)));

        EXPECT_EQ(a1["0"], a1[0]);
        EXPECT_EQ(4, a1[0].asLong());

        EXPECT_EQ(phpcxx::Type::Null, a1[1].type());
    });
    EXPECT_EQ("", m_err.str());
    m_err.str(std::string());
}

TEST_F(ArrayFixture, InitializationVectorMap)
{
    m_sapi.run([this]() {
        phpcxx::vector<int> v = {0, 1, 2, 3, 4};
        phpcxx::map<int, double> m = { { 0, 0.0 }, { 1, 1.1 }, { 2, 2.2 }, { 3, 3.3 }, { 4, 4.4 } };
        phpcxx::map<const char*, double> n = { { "k5", 5.5 }, { "k6", 6.6 }, { "k7", 7.7 }, { "k8", 8.8 }, { "k9", 9.9 } };
        phpcxx::Array a;

        a = v;

        EXPECT_EQ(5, a.size());
        for (int i=0; i<5; ++i) {
            EXPECT_EQ(phpcxx::Type::Integer, a[i].type());
            EXPECT_EQ(i, a[i].asLong());
        }

        a = m;
        EXPECT_EQ(5, a.size());
        for (int i=0; i<5; ++i) {
            EXPECT_EQ(phpcxx::Type::Double, a[i].type());
            EXPECT_EQ(i + i/10.0, a[i].asDouble());
        }

        a = n;
        EXPECT_EQ(5, a.size());
        for (int i=5; i<10; ++i) {
            phpcxx::string key = "k" + phpcxx::string(std::to_string(i).c_str());
            EXPECT_EQ(phpcxx::Type::Double, a[key].type());
            EXPECT_EQ(i + i/10.0, a[key].asDouble());
        }
    });

    EXPECT_EQ("", m_err.str());
    m_err.str(std::string());
}

TEST_F(ArrayFixture, Operations)
{
    m_sapi.run([this]() {
        phpcxx::Array a;
        phpcxx::Value zero(zend_long(0));
        phpcxx::Value one(1);

        EXPECT_EQ(phpcxx::Type::Integer, zero.type());
        EXPECT_EQ(phpcxx::Type::Integer, one.type());
        EXPECT_EQ(0, Z_LVAL_P(zero.pzval()));
        EXPECT_EQ(1, Z_LVAL_P(one.pzval()));

        EXPECT_EQ(0, a.size());

        a[nullptr] = 1;
        EXPECT_EQ(1, a.size());
        EXPECT_TRUE(a.isset("0"));
        EXPECT_TRUE(a.isset(phpcxx::string("0")));
        EXPECT_TRUE(a.isset(zend_long(0)));
        EXPECT_TRUE(a.isset(zero));

        EXPECT_EQ(a[zero], a["0"]);
        EXPECT_EQ(a[phpcxx::string("0")], a["0"]);
        EXPECT_EQ(a[zend_long(0)], a["0"]);
        EXPECT_EQ(one, a["0"]);

        EXPECT_FALSE(a.isset("1"));
        EXPECT_FALSE(a.isset(phpcxx::string("1")));
        EXPECT_FALSE(a.isset(1));
        EXPECT_FALSE(a.isset(one));

        phpcxx::Array b(a);
        b.unset(zero);

        EXPECT_EQ(0, b.size());
        EXPECT_EQ(1, a.size());

        b = a;

        EXPECT_EQ(1, b.size());
        EXPECT_TRUE(b.isset(zero));
        EXPECT_EQ(a["0"], b["0"]);

        b.unset("0");

        EXPECT_EQ(0, b.size());
        EXPECT_EQ(1, a.size());

        b = a;

        EXPECT_EQ(1, b.size());
        EXPECT_TRUE(b.isset(zero));
        EXPECT_EQ(a["0"], b["0"]);

        b.unset(phpcxx::string("0"));

        EXPECT_EQ(0, b.size());
        EXPECT_EQ(1, a.size());

        zero *= 1.0;
        EXPECT_EQ(phpcxx::Type::Double, zero.type());
        EXPECT_TRUE(a.isset(zero));

        zero = false;
        EXPECT_EQ(phpcxx::Type::False, zero.type());
        EXPECT_TRUE(a.isset(zero));

        a.unset(0);
        EXPECT_FALSE(a.isset(zero));
    });

    EXPECT_EQ("", m_err.str());
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
    EXPECT_EQ("", m_err.str());
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
    EXPECT_EQ("", m_err.str());
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
    EXPECT_EQ("", m_err.str());
    m_err.str(std::string());

    /**/
    m_sapi.run([this]() {
        phpcxx::Array a;
        a["a"]["b"]["c"] = 1;
        a["a"]["b"]["d"] = 2;
        a["a"]["e"]      = 3;
        phpcxx::Value func("var_export");
        func(a);

        EXPECT_TRUE(a.isset("a"));
        EXPECT_FALSE(a.isset("b"));
        EXPECT_EQ(1, a.size());

        phpcxx::Array aa(a["a"]);
        phpcxx::Value e("e");
        EXPECT_TRUE(aa.isset(phpcxx::string("b")));
        EXPECT_TRUE(aa.isset(e));
        EXPECT_EQ(2, aa.size());

        a["f"] = "smth";
        EXPECT_TRUE(a.isset("f"));
        EXPECT_EQ(2, a.size());
        a.unset("f");
        EXPECT_FALSE(a.isset("f"));
        EXPECT_EQ(1, a.size());

        std::string expected = "array (\n  'a' => \n  array (\n    'b' => \n    array (\n      'c' => 1,\n      'd' => 2,\n    ),\n    'e' => 3,\n  ),\n)";
        std::string actual   = m_out.str();
        m_out.str(std::string());

        EXPECT_EQ(expected, actual);
    });
    EXPECT_EQ("", m_err.str());
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
    EXPECT_EQ("", m_err.str());
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
    EXPECT_EQ("", m_err.str());
    m_err.str(std::string());
}

TEST_F(ArrayFixture, TestIsset)
{
    m_sapi.run([]() {
        phpcxx::Array a;

        phpcxx::Value v0  = 0;
        phpcxx::Value v1  = 1;
        phpcxx::string s0 = "0";
        phpcxx::string s1 = "1";
        const char* c0    = "0";
        const char* c1    = "1";
        phpcxx::ZendString z0("0");
        phpcxx::ZendString z1("1");

        phpcxx::Value v_unset;
        phpcxx::Value v_null(nullptr);
        phpcxx::Value v_false(false);
        phpcxx::Value v_true(true);

        a[0] = 1;

        EXPECT_TRUE(a.isset(v0));
        EXPECT_TRUE(a.isset(s0));
        EXPECT_TRUE(a.isset(c0));
        EXPECT_TRUE(a.isset(0));
        EXPECT_TRUE(a.isset(v_false));
        EXPECT_TRUE(a.isset(z0));

        EXPECT_FALSE(a.isset(v1));
        EXPECT_FALSE(a.isset(s1));
        EXPECT_FALSE(a.isset(c1));
        EXPECT_FALSE(a.isset(1));
        EXPECT_FALSE(a.isset(v_true));
        EXPECT_FALSE(a.isset(v_null));
        EXPECT_FALSE(a.isset(v_unset));
        EXPECT_FALSE(a.isset(z1));

        phpcxx::Value ref = v0.reference();
        EXPECT_TRUE(a.isset(ref));

        EXPECT_THROW(a.isset(phpcxx::Value(a.pzval())), std::invalid_argument);

        zval fake_res;
        ZVAL_NEW_RES(&fake_res, 1, nullptr, 1);
        {
            phpcxx::Value fr(&fake_res);
            EXPECT_FALSE(a.isset(fr));
        }
        efree(Z_RES(fake_res));
    });

    EXPECT_EQ("", m_err.str());
    m_err.str(std::string());
}

TEST_F(ArrayFixture, TestUnset)
{
    m_sapi.run([]() {
        phpcxx::Array a;

        phpcxx::string s0 = "0";
        const char* c1    = "1";
        phpcxx::Value v2  = 3;
        phpcxx::ZendString z3("3");
        phpcxx::Value v4  = 4.0;
        phpcxx::Value v5  = "5";
        phpcxx::Value r5  = v5.reference();

        phpcxx::Value v_unset;
        phpcxx::Value v_null(nullptr);
        phpcxx::Value v_false(false);
        phpcxx::Value v_true(true);

        a[s0] = 0;
        a[c1] = 1;
        a[v2] = 2;
        a[z3] = 3;
        a[v4] = 4;
        a[v5] = 5;

        EXPECT_TRUE(a.isset(s0));
        EXPECT_TRUE(a.isset(c1));
        EXPECT_TRUE(a.isset(v2));
        EXPECT_TRUE(a.isset(z3));
        EXPECT_TRUE(a.isset(v4));
        EXPECT_TRUE(a.isset(v5));
        EXPECT_TRUE(a.isset(r5));

        EXPECT_TRUE(a.isset(v_false));
        EXPECT_TRUE(a.isset(v_true));
        EXPECT_FALSE(a.isset(v_null));
        EXPECT_FALSE(a.isset(v_unset));

        a.unset(s0);
        a.unset(c1);
        a.unset(v2);
        a.unset(z3);
        a.unset(v4);
        a.unset(r5);

        EXPECT_FALSE(a.isset(s0));
        EXPECT_FALSE(a.isset(c1));
        EXPECT_FALSE(a.isset(v2));
        EXPECT_FALSE(a.isset(z3));
        EXPECT_FALSE(a.isset(v4));
        EXPECT_FALSE(a.isset(v5));
        EXPECT_FALSE(a.isset(r5));

        EXPECT_FALSE(a.isset(v_false));
        EXPECT_FALSE(a.isset(v_true));
        EXPECT_FALSE(a.isset(v_null));
        EXPECT_FALSE(a.isset(v_unset));

        EXPECT_THROW(a.unset(phpcxx::Value(a.pzval())), std::invalid_argument);

        zval fake_res;
        ZVAL_NEW_RES(&fake_res, 1, nullptr, 1);
        {
            phpcxx::Value fr(&fake_res);

            a[v_true] = 1;
            EXPECT_TRUE(a.isset(1));
            EXPECT_TRUE(a.isset(v_true));
            EXPECT_TRUE(a.isset(fr));
            a.unset(fr);
            EXPECT_FALSE(a.isset(1));
            EXPECT_FALSE(a.isset(v_true));
            EXPECT_FALSE(a.isset(fr));

            a[fr] = 1;
            EXPECT_TRUE(a.isset(1));
            EXPECT_TRUE(a.isset(v_true));
            EXPECT_TRUE(a.isset(fr));
            a.unset(v_true);
            EXPECT_FALSE(a.isset(1));
            EXPECT_FALSE(a.isset(v_true));
            EXPECT_FALSE(a.isset(fr));
        }
        efree(Z_RES(fake_res));

        a[v_false] = 0;
        EXPECT_TRUE(a.isset(v_false));
        EXPECT_FALSE(a.isset(v_null));
        EXPECT_FALSE(a.isset(v_unset));
        a.unset(v_false);
        EXPECT_FALSE(a.isset(v_false));

        a[v_unset] = 0;
        EXPECT_TRUE(a.isset(v_unset));
        EXPECT_TRUE(a.isset(v_null));
        EXPECT_TRUE(a.isset(""));
        EXPECT_FALSE(a.isset(v_false));
        a.unset(v_unset);
        EXPECT_FALSE(a.isset(v_unset));
        EXPECT_FALSE(a.isset(v_null));
        EXPECT_FALSE(a.isset(""));

        a[v_null] = 0;
        EXPECT_TRUE(a.isset(v_unset));
        EXPECT_TRUE(a.isset(v_null));
        EXPECT_TRUE(a.isset(""));
        EXPECT_FALSE(a.isset(v_false));
        a.unset(v_null);
        EXPECT_FALSE(a.isset(v_unset));
        EXPECT_FALSE(a.isset(v_null));
        EXPECT_FALSE(a.isset(""));
    });

    EXPECT_EQ("", m_err.str());
    m_err.str(std::string());
}

TEST_F(ArrayFixture, TestExceptions)
{
    m_sapi.run([]() {
        phpcxx::Array a;

        zval z;
        ZVAL_PTR(&z, nullptr);

        phpcxx::Value idx(&z);

        EXPECT_THROW(a[idx],       std::invalid_argument);
        EXPECT_THROW(a.isset(idx), std::invalid_argument);
        EXPECT_THROW(a.unset(idx), std::invalid_argument);
    });

    EXPECT_EQ("", m_err.str());
    m_err.str(std::string());
}

TEST_F(ArrayFixture, TestGlobals)
{
    m_sapi.run([]() {
        zval z;
        runPhpCode("global $a; global $b; $b = $GLOBALS['GLOBALS'];");
        runPhpCode("$b;", z);

        ASSERT_EQ(IS_ARRAY, Z_TYPE(z));

        phpcxx::Array a(&z);
        EXPECT_TRUE(a.isset("a"));
        EXPECT_TRUE(a.isset("b"));
        a.unset("a");
        EXPECT_FALSE(a.isset("a"));
    });

    EXPECT_EQ("", m_err.str());
    m_err.str(std::string());
}

TEST_F(ArrayFixture, TestIndirect)
{
    m_sapi.run([]() {
        zval y;
        zval z;
        ZVAL_TRUE(&z);
        ZVAL_INDIRECT(&y, &z);

        phpcxx::Array a;
        a["idx"] = &y;

        // Indirect variables will be resolved
        phpcxx::Value vy = a["idx"];
        EXPECT_EQ(phpcxx::Type::True, vy.type());

        ZVAL_UNDEF(&z);

        // Indirect IS_UNDEF variables will be returned as IS_NULL
        vy = a["idx"];
        EXPECT_EQ(phpcxx::Type::Null, vy.type());

        // Indirection is not resolved for numeric indices
        a[0] = &y;
        vy = a[0];
        EXPECT_EQ(phpcxx::Type::Indirect, vy.type());
    });

    EXPECT_EQ("", m_err.str());
    m_err.str(std::string());
}

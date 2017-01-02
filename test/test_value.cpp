#include <sstream>
#include <gtest/gtest.h>
#include "phpcxx/value.h"
#include "globals.h"
#include "testsapi.h"

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

typedef CommonFixture ValueFixture;

TEST_F(ValueFixture, Initialization)
{
    m_sapi.run([]() {
        phpcxx::Value undefined;
        EXPECT_EQ(phpcxx::Type::Undefined, undefined.type());
        EXPECT_EQ(0, undefined.refCount());
        EXPECT_FALSE(undefined.isReference());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value null(nullptr);
        EXPECT_EQ(phpcxx::Type::Null, null.type());
        EXPECT_EQ(0, null.refCount());
        EXPECT_FALSE(null.isReference());

        phpcxx::Value v;
        EXPECT_EQ(phpcxx::Type::Undefined, v.type());
        v = nullptr;
        EXPECT_EQ(phpcxx::Type::Null, v.type());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value integer(123);
        EXPECT_EQ(phpcxx::Type::Integer, integer.type());
        EXPECT_EQ(0, integer.refCount());
        EXPECT_FALSE(integer.isReference());
        EXPECT_EQ(123, integer.asLong());

        phpcxx::Value v;
        v = 456;
        EXPECT_EQ(phpcxx::Type::Integer, v.type());
        EXPECT_EQ(456, v.asLong());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value integer(123l);
        EXPECT_EQ(phpcxx::Type::Integer, integer.type());
        EXPECT_EQ(0, integer.refCount());
        EXPECT_FALSE(integer.isReference());
        EXPECT_EQ(123, integer.asLong());

        phpcxx::Value v;
        v = 456l;
        EXPECT_EQ(phpcxx::Type::Integer, v.type());
        EXPECT_EQ(456, v.asLong());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value f(123.0f);
        phpcxx::Value d(123.0);
        phpcxx::Value ld(123.0l);
        EXPECT_EQ(phpcxx::Type::Double, f.type());
        EXPECT_EQ(phpcxx::Type::Double, d.type());
        EXPECT_EQ(phpcxx::Type::Double, ld.type());
        EXPECT_EQ(0, f.refCount());
        EXPECT_EQ(0, d.refCount());
        EXPECT_EQ(0, ld.refCount());
        EXPECT_FALSE(f.isReference());
        EXPECT_FALSE(d.isReference());
        EXPECT_FALSE(ld.isReference());
        EXPECT_EQ(123.0, f.asDouble());
        EXPECT_EQ(123.0, d.asDouble());
        EXPECT_EQ(123.0, ld.asDouble());

        phpcxx::Value v;
        v = 456.0l;
        EXPECT_EQ(phpcxx::Type::Double, v.type());
        EXPECT_EQ(456.0, v.asDouble());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value s("test");
        phpcxx::Value t(phpcxx::string("test"));
        EXPECT_EQ(phpcxx::Type::String, s.type());
        EXPECT_EQ(phpcxx::Type::String, t.type());
        EXPECT_EQ(1, s.refCount());
        EXPECT_EQ(1, t.refCount());
        EXPECT_TRUE(s.isCopyable());
        EXPECT_TRUE(s.isRefcounted());
        EXPECT_FALSE(s.isReference());
        EXPECT_EQ(s.asString(), t.toString());
        EXPECT_EQ("test", s.asString());

        phpcxx::Value v;
        phpcxx::Value w;
        v = "something";
        w = phpcxx::string("anything");
        EXPECT_EQ(phpcxx::Type::String, v.type());
        EXPECT_EQ(phpcxx::Type::String, w.type());
        EXPECT_EQ("something", v.asString());
        EXPECT_EQ("anything",  w.asString());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
    /*
        [type=6 refcounted=1 isref=0 refcount=3] s
        [type=6 refcounted=1 isref=0 refcount=3] s
        [type=6 refcounted=1 isref=0 refcount=3] s
     */
        phpcxx::Value a = "s";
        phpcxx::Value b = a;
        phpcxx::Value c = a;

        EXPECT_EQ(phpcxx::Type::String, a.type());
        EXPECT_EQ(phpcxx::Type::String, b.type());
        EXPECT_EQ(phpcxx::Type::String, c.type());
        EXPECT_FALSE(a.isReference());
        EXPECT_FALSE(b.isReference());
        EXPECT_FALSE(c.isReference());
        EXPECT_EQ(3, a.refCount());
        EXPECT_EQ(3, b.refCount());
        EXPECT_EQ(3, c.refCount());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value a = "somestring";
        phpcxx::Value b(a, phpcxx::CopyPolicy::Assign);
        phpcxx::Value c(a, phpcxx::CopyPolicy::Copy);
        phpcxx::Value d(a, phpcxx::CopyPolicy::Reference);

        EXPECT_EQ("somestring", a.toString());
        EXPECT_EQ("somestring", b.toString());
        EXPECT_EQ("somestring", c.toString());
        EXPECT_EQ("somestring", d.toString());

        a = "otherstring";
        EXPECT_EQ("otherstring", a.toString());
        EXPECT_EQ("somestring",  b.toString());
        EXPECT_EQ("somestring",  c.toString());
        EXPECT_EQ("otherstring", d.toString());
    });
}

TEST_F(ValueFixture, References)
{
    m_sapi.run([]() {
        /*
            $a = 4;
            $b = &$a;
            xdebug_debug_zval('a', 'b');

            a: (refcount=2, is_ref=1)=4
            b: (refcount=2, is_ref=1)=4
         */
        phpcxx::Value a(4);
        phpcxx::Value b = phpcxx::Value::createReference(a);

        EXPECT_TRUE(a.isReference());
        EXPECT_TRUE(b.isReference());
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(2, b.refCount());
        EXPECT_EQ(4, a.asLong());
        EXPECT_EQ(4, b.asLong());
        EXPECT_TRUE(a.isReference());
        EXPECT_TRUE(b.isReference());

        /*
            $a = 5;
            xdebug_debug_zval('a', 'b');

            a: (refcount=2, is_ref=1)=5
            b: (refcount=2, is_ref=1)=5
         */
        a = 5;
        EXPECT_TRUE(a.isReference());
        EXPECT_TRUE(b.isReference());
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(2, b.refCount());
        EXPECT_EQ(5, a.asLong());
        EXPECT_EQ(5, b.asLong());

        /*
            $b = 6;
            xdebug_debug_zval('a', 'b');

            a: (refcount=2, is_ref=1)=6
            b: (refcount=2, is_ref=1)=6
         */
        b = 6;
        EXPECT_TRUE(a.isReference());
        EXPECT_TRUE(b.isReference());
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(2, b.refCount());
        EXPECT_EQ(6, a.asLong());
        EXPECT_EQ(6, b.asLong());

        /*
            $b = $a;
            xdebug_debug_zval('a', 'b');

            a: (refcount=2, is_ref=1)=6
            b: (refcount=2, is_ref=1)=6
         */
        b = a;
        EXPECT_TRUE(a.isReference());
        EXPECT_TRUE(b.isReference());
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(2, b.refCount());
        EXPECT_EQ(6, a.asLong());
        EXPECT_EQ(6, b.asLong());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        /*
            $a = 0;
            $b = &$a;
            $c = $b;
            $a = 1;

            a: (refcount=2, is_ref=1)=1
            b: (refcount=2, is_ref=1)=1
            c: (refcount=0, is_ref=0)=0
         */
        phpcxx::Value a = 0;
        phpcxx::Value b = phpcxx::Value::createReference(a);
        phpcxx::Value c = b;
        a = 1;

        EXPECT_TRUE(a.isReference());
        EXPECT_TRUE(b.isReference());
        EXPECT_EQ(phpcxx::Type::Integer, c.type());
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(2, b.refCount());
        EXPECT_EQ(0, c.refCount());
        EXPECT_EQ(1, a.asLong());
        EXPECT_EQ(1, b.asLong());
        EXPECT_EQ(0, c.asLong());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([]() {
        /*
            $a = 0;
            $b = &$a;
            $c = $b;
            $a = 's';
            $b = $a;
            $c = $a;

            a: (refcount=2, is_ref=1)='s'
            b: (refcount=2, is_ref=1)='s'
            c: (refcount=2, is_ref=0)='s'
         */
        phpcxx::Value a = 0;
        phpcxx::Value b = phpcxx::Value::createReference(a);
        phpcxx::Value c = b;
        a = "s";
        b = a;
        c = a;

        EXPECT_TRUE(a.isReference());
        EXPECT_TRUE(b.isReference());
        EXPECT_EQ(phpcxx::Type::String, c.type());
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(b.refCount(), c.refCount());
        EXPECT_EQ(2, c.refCount());
        EXPECT_EQ("s", a.asString());
        EXPECT_EQ("s", b.asString());
        EXPECT_EQ("s", c.asString());
    });
    EXPECT_EQ(m_err.str(), "");
    m_err.str(std::string());

    m_sapi.run([this]() {
        {
            // arr: (refcount=1, is_ref=0)=array (0 => (refcount=0, is_ref=0)=4)
            // a: (refcount=2, is_ref=1)=4
            // b: (refcount=2, is_ref=1)=4

            phpcxx::Value arr;
            phpcxx::Value a = 4;
            phpcxx::Value b = phpcxx::Value::createReference(a);

            arr[nullptr] = a;

            EXPECT_EQ(phpcxx::Type::Array, arr.type());
            EXPECT_EQ(1, arr.refCount());
            EXPECT_TRUE(a.isReference());
            EXPECT_TRUE(b.isReference());
            EXPECT_EQ(2, a.refCount());
            EXPECT_EQ(2, b.refCount());

            zval* z = arr.getZVal();
            zval* x = zend_hash_index_find(Z_ARRVAL_P(z), 0);
            ASSERT_TRUE(x != nullptr);
            ASSERT_EQ(IS_LONG, Z_TYPE_P(x));
            EXPECT_EQ(4, Z_LVAL_P(x));
        }

        EXPECT_EQ(m_err.str(), "");
        m_err.str(std::string());

        {
            // arr: (refcount=1, is_ref=0)=array (0 => (refcount=3, is_ref=1)=4)
            // a: (refcount=3, is_ref=1)=4
            // b: (refcount=3, is_ref=1)=4

            phpcxx::Value arr;
            phpcxx::Value a = 4;
            phpcxx::Value b = phpcxx::Value::createReference(a);

            arr[nullptr] = phpcxx::Value::createReference(a);

            EXPECT_EQ(phpcxx::Type::Array, arr.type());
            EXPECT_EQ(1, arr.refCount());
            EXPECT_TRUE(a.isReference());
            EXPECT_TRUE(b.isReference());
            EXPECT_EQ(3, a.refCount());
            EXPECT_EQ(3, b.refCount());

            zval* z = arr.getZVal();
            zval* x = zend_hash_index_find(Z_ARRVAL_P(z), 0);
            ASSERT_TRUE(x != nullptr);
            ASSERT_EQ(IS_REFERENCE, Z_TYPE_P(x));
            ASSERT_EQ(IS_LONG, Z_TYPE_P(Z_REFVAL_P(x)));
            EXPECT_EQ(4, Z_LVAL_P(Z_REFVAL_P(x)));
        }

        EXPECT_EQ(m_err.str(), "");
        m_err.str(std::string());
        EXPECT_TRUE(m_out.str().empty());
        m_out.str(std::string());
    });
}

TEST_F(ValueFixture, Arrays)
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
}

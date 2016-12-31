#include <sstream>
#include <gtest/gtest.h>
#include "phpcxx/value.h"
#include "testsapi.h"

class CommonFixture : public ::testing::Test {
protected:
    std::stringstream m_out;
    std::stringstream m_err;
    TestSAPI m_sapi;

    CommonFixture()
        : m_out(), m_err(), m_sapi(m_out, m_err)
    {
    }
};

typedef CommonFixture ValueFixture;

TEST_F(ValueFixture, Initialization)
{
    m_sapi.run([]() {
        phpcxx::Value undefined;
        EXPECT_EQ(undefined.type(), phpcxx::Type::Undefined);
        EXPECT_EQ(undefined.refCount(), 0);
        EXPECT_EQ(undefined.isReference(), false);
    });
    EXPECT_TRUE(m_err.str().empty());
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value null(nullptr);
        EXPECT_EQ(null.type(), phpcxx::Type::Null);
        EXPECT_EQ(null.refCount(), 0);
        EXPECT_EQ(null.isReference(), false);

        phpcxx::Value v;
        EXPECT_EQ(v.type(), phpcxx::Type::Undefined);
        v = nullptr;
        EXPECT_EQ(v.type(), phpcxx::Type::Null);
    });
    EXPECT_TRUE(m_err.str().empty());
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value integer(123);
        EXPECT_EQ(integer.type(), phpcxx::Type::Integer);
        EXPECT_EQ(integer.refCount(), 0);
        EXPECT_EQ(integer.isReference(), false);
        EXPECT_EQ(integer.asLong(), 123);

        phpcxx::Value v;
        v = 456;
        EXPECT_EQ(v.type(), phpcxx::Type::Integer);
        EXPECT_EQ(v.asLong(), 456);
    });
    EXPECT_TRUE(m_err.str().empty());
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value integer(123l);
        EXPECT_EQ(integer.type(), phpcxx::Type::Integer);
        EXPECT_EQ(integer.refCount(), 0);
        EXPECT_EQ(integer.isReference(), false);
        EXPECT_EQ(integer.asLong(), 123);

        phpcxx::Value v;
        v = 456;
        EXPECT_EQ(v.type(), phpcxx::Type::Integer);
        EXPECT_EQ(v.asLong(), 456);

        v = 456l;
        EXPECT_EQ(v.type(), phpcxx::Type::Integer);
        EXPECT_EQ(v.asLong(), 456);
    });
    EXPECT_TRUE(m_err.str().empty());
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value f(123.0f);
        phpcxx::Value d(123.0);
        phpcxx::Value ld(123.0l);
        EXPECT_EQ(f.type(), phpcxx::Type::Double);
        EXPECT_EQ(d.type(), phpcxx::Type::Double);
        EXPECT_EQ(ld.type(), phpcxx::Type::Double);
        EXPECT_EQ(f.refCount(), 0);
        EXPECT_EQ(d.refCount(), 0);
        EXPECT_EQ(ld.refCount(), 0);
        EXPECT_EQ(f.isReference(), false);
        EXPECT_EQ(d.isReference(), false);
        EXPECT_EQ(ld.isReference(), false);
        EXPECT_EQ(f.asDouble(), 123.0);
        EXPECT_EQ(d.asDouble(), 123.0);
        EXPECT_EQ(ld.asDouble(), 123.0);

        phpcxx::Value v;
        v = 456.0l;
        EXPECT_EQ(v.type(), phpcxx::Type::Double);
        EXPECT_EQ(v.asDouble(), 456.0);
    });
    EXPECT_TRUE(m_err.str().empty());
    m_err.str(std::string());

    m_sapi.run([]() {
        phpcxx::Value s("test");
        phpcxx::Value t(phpcxx::string("test"));
        EXPECT_EQ(s.type(), phpcxx::Type::String);
        EXPECT_EQ(t.type(), phpcxx::Type::String);
        EXPECT_EQ(s.refCount(), 1);
        EXPECT_EQ(t.refCount(), 1);
        EXPECT_TRUE(s.isCopyable());
        EXPECT_TRUE(s.isRefcounted());
        EXPECT_FALSE(s.isReference());
        EXPECT_EQ(s.asString(), t.toString());
        EXPECT_EQ(s.asString(), "test");

        phpcxx::Value v;
        phpcxx::Value w;
        v = "something";
        w = phpcxx::string("anything");
        EXPECT_EQ(v.type(), phpcxx::Type::String);
        EXPECT_EQ(w.type(), phpcxx::Type::String);
        EXPECT_EQ(v.asString(), "something");
        EXPECT_EQ(w.asString(), "anything");
    });
    EXPECT_TRUE(m_err.str().empty());
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

        EXPECT_EQ(a.type(), phpcxx::Type::String);
        EXPECT_EQ(b.type(), phpcxx::Type::String);
        EXPECT_EQ(c.type(), phpcxx::Type::String);
        EXPECT_FALSE(a.isReference());
        EXPECT_FALSE(b.isReference());
        EXPECT_FALSE(c.isReference());
        EXPECT_EQ(a.refCount(), 3);
        EXPECT_EQ(b.refCount(), 3);
        EXPECT_EQ(c.refCount(), 3);
    });
    EXPECT_TRUE(m_err.str().empty());
    m_err.str(std::string());
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

        EXPECT_EQ(a.type(), phpcxx::Type::Reference);
        EXPECT_EQ(b.type(), phpcxx::Type::Reference);
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(b.refCount(), 2);
        EXPECT_EQ(a.asLong(), 4);
        EXPECT_EQ(b.asLong(), 4);
        EXPECT_EQ(a.type(), phpcxx::Type::Reference);
        EXPECT_EQ(b.type(), phpcxx::Type::Reference);

        /*
            $a = 5;
            xdebug_debug_zval('a', 'b');

            a: (refcount=2, is_ref=1)=5
            b: (refcount=2, is_ref=1)=5
         */
        a = 5;
        EXPECT_EQ(a.type(), phpcxx::Type::Reference);
        EXPECT_EQ(b.type(), phpcxx::Type::Reference);
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(b.refCount(), 2);
        EXPECT_EQ(a.asLong(), 5);
        EXPECT_EQ(b.asLong(), 5);

        /*
            $b = 6;
            xdebug_debug_zval('a', 'b');

            a: (refcount=2, is_ref=1)=6
            b: (refcount=2, is_ref=1)=6
         */
        b = 6;
        EXPECT_EQ(a.type(), phpcxx::Type::Reference);
        EXPECT_EQ(b.type(), phpcxx::Type::Reference);
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(b.refCount(), 2);
        EXPECT_EQ(a.asLong(), 6);
        EXPECT_EQ(b.asLong(), 6);

        /*
            $b = $a;
            xdebug_debug_zval('a', 'b');

            a: (refcount=2, is_ref=1)=6
            b: (refcount=2, is_ref=1)=6
         */
        b = a;
        EXPECT_EQ(a.type(), phpcxx::Type::Reference);
        EXPECT_EQ(b.type(), phpcxx::Type::Reference);
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(b.refCount(), 2);
        EXPECT_EQ(a.asLong(), 6);
        EXPECT_EQ(b.asLong(), 6);
    });
    EXPECT_TRUE(m_err.str().empty());
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

        EXPECT_EQ(a.type(), phpcxx::Type::Reference);
        EXPECT_EQ(b.type(), phpcxx::Type::Reference);
        EXPECT_EQ(c.type(), phpcxx::Type::Integer);
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(b.refCount(), 2);
        EXPECT_EQ(c.refCount(), 0);
        EXPECT_EQ(a.asLong(), 1);
        EXPECT_EQ(b.asLong(), 1);
        EXPECT_EQ(c.asLong(), 0);
    });

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

        EXPECT_EQ(a.type(), phpcxx::Type::Reference);
        EXPECT_EQ(b.type(), phpcxx::Type::Reference);
        EXPECT_EQ(c.type(), phpcxx::Type::String);
        EXPECT_EQ(a.refCount(), b.refCount());
        EXPECT_EQ(b.refCount(), c.refCount());
        EXPECT_EQ(c.refCount(), 2);
        EXPECT_EQ(a.asString(), "s");
        EXPECT_EQ(b.asString(), "s");
        EXPECT_EQ(c.asString(), "s");
    });
}

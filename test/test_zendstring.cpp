#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "phpcxx/module.h"
#include "phpcxx/zendstring.h"
#include "testsapi.h"

extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_string.h>
}

TEST(ZendString, TestConstruct)
{
    phpcxx::Module module("ZendStrings", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([] {
            phpcxx::ZendString zs1("c string");
            phpcxx::ZendString zs2(std::string("std::string"));
            phpcxx::ZendString zs3(phpcxx::string("phpcxx::string"));
#if PHP_VERSION_ID < 70200
            phpcxx::ZendString zs4(CG(empty_string));
#else
            phpcxx::ZendString zs4(zend_empty_string);
#endif

            EXPECT_EQ(1, zs1.refCount());
            EXPECT_EQ(1, zs2.refCount());
            EXPECT_EQ(1, zs3.refCount());
            EXPECT_EQ(1, zs4.refCount());

            EXPECT_FALSE(zs1.isInterned());
            EXPECT_FALSE(zs2.isInterned());
            EXPECT_FALSE(zs3.isInterned());
            EXPECT_TRUE(zs4.isInterned());

            zend_string* z = zs1.get();
            EXPECT_EQ(1, zs1.refCount());
            EXPECT_EQ(1, zend_string_refcount(z));

            z = zs1.release();
            EXPECT_EQ(0, zs1.refCount());
            EXPECT_EQ(1, zend_string_refcount(z));
            zend_string_release(z);

            phpcxx::ZendString copied(zs2);
            EXPECT_EQ(2, zs2.refCount());
            EXPECT_EQ(2, copied.refCount());
            EXPECT_TRUE(zs2 == copied);

            phpcxx::ZendString moved(std::move(zs3));
            EXPECT_EQ(0, zs3.refCount());
            EXPECT_EQ(1, moved.refCount());
        });
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);
}

TEST(ZendString, TestAssign)
{
    phpcxx::Module module("ZendStrings", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([] {
#if PHP_VERSION_ID < 70200
            zend_string* empty = CG(empty_string);
#else
            zend_string* empty = zend_empty_string;
#endif
            phpcxx::ZendString zs1(empty);
            phpcxx::ZendString zs2(empty);
            phpcxx::ZendString zs3(empty);
            phpcxx::ZendString zs4(empty);

            zs1 = "c string";
            zs2 = std::string("std::string");
            zs3 = phpcxx::string("phpcxx::string");
            zs4 = empty;

            EXPECT_EQ(1, zs1.refCount());
            EXPECT_EQ(1, zs2.refCount());
            EXPECT_EQ(1, zs3.refCount());
            EXPECT_EQ(1, zs4.refCount());

            EXPECT_FALSE(zs1.isInterned());
            EXPECT_FALSE(zs2.isInterned());
            EXPECT_FALSE(zs3.isInterned());
            EXPECT_TRUE(zs4.isInterned());

            zend_string* z = zs1.get();
            EXPECT_EQ(1, zs1.refCount());
            EXPECT_EQ(1, zend_string_refcount(z));

            z = zs1.release();
            EXPECT_EQ(0, zs1.refCount());
            EXPECT_EQ(1, zend_string_refcount(z));
            zend_string_release(z);

            zs1 = zs2;
            zs3 = zs2;
            EXPECT_EQ(3, zs1.refCount());
            EXPECT_EQ(3, zs2.refCount());
            EXPECT_EQ(3, zs3.refCount());

            zs4 = std::move(zs1);
            EXPECT_EQ(0, zs1.refCount());
            EXPECT_EQ(3, zs2.refCount());
            EXPECT_EQ(3, zs3.refCount());
            EXPECT_EQ(3, zs4.refCount());

            zs4 = empty;
            EXPECT_TRUE(zs4.isInterned());
            zs4.makeInterned();
            EXPECT_TRUE(zs4.isInterned());
        });
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);
}

TEST(ZendString, TestReferences)
{
    phpcxx::Module module("ZendStrings", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([] {
#if PHP_VERSION_ID < 70200
            zend_string* empty = CG(empty_string);
#else
            zend_string* empty = zend_empty_string;
#endif

            // Interned strings always have refcount = 1
            // Interned strings are not destroyed
            EXPECT_EQ(1, zend_string_refcount(empty));
            {
                phpcxx::ZendString zs(empty);

                EXPECT_EQ(1, zs.refCount());
                EXPECT_TRUE(zs.isInterned());
                EXPECT_EQ(1, zs.addRef());
                EXPECT_EQ(1, zs.delRef());

                EXPECT_EQ(empty, zs.get());
            }
            EXPECT_EQ(1, zend_string_refcount(empty));

            zend_string* z = zend_string_init(ZEND_STRL("test"), 0);
            EXPECT_EQ(1, zend_string_refcount(z));
            {
                phpcxx::ZendString zs = z;

                EXPECT_EQ(2, zend_string_refcount(z));
                ASSERT_EQ(2, zs.refCount());
            }
            ASSERT_EQ(1, zend_string_refcount(z));
            zend_string_release(z);

            {
                phpcxx::ZendString zs("test");
                z = zs.get();

                EXPECT_EQ(1, zend_string_refcount(z));
                ASSERT_EQ(1, zs.refCount());

                EXPECT_EQ(2, zs.addRef());
                EXPECT_EQ(2, zend_string_refcount(z));
            }
            ASSERT_EQ(1, zend_string_refcount(z));
            zend_string_release(z);

            z = zend_string_init(ZEND_STRL("test"), 0);
            zend_string_addref(z);
            EXPECT_EQ(2, zend_string_refcount(z));
            zend_string_addref(z);
            EXPECT_EQ(3, zend_string_refcount(z));
            {
                phpcxx::ZendString zs = z;

                EXPECT_EQ(4, zend_string_refcount(z));
                EXPECT_EQ(4, zs.refCount());

                EXPECT_EQ(3, zs.delRef());
                EXPECT_EQ(3, zs.refCount());
            }
            EXPECT_EQ(2, zend_string_refcount(z));
            zend_string_release(z);
            ASSERT_EQ(1, zend_string_refcount(z));
            zend_string_release(z);
        });
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);
}

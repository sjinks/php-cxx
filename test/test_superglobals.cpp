#include <cstring>
#include <sstream>
#include <gtest/gtest.h>
#include "phpcxx/array.h"
#include "phpcxx/superglobal.h"
#include "phpcxx/module.h"
#include "phpcxx/value.h"
#include "globals.h"
#include "testsapi.h"

extern "C" {
#include <Zend/zend_compile.h>
}

namespace {

class MyModule : public phpcxx::Module {
public:
    using phpcxx::Module::Module;

protected:

    virtual bool moduleStartup() override
    {
        zend_register_auto_global(zend_string_init(ZEND_STRL("SUPERGLOBAL_INT"), 1), 1, &MyModule::rag_callback);
        zend_register_auto_global(zend_string_init(ZEND_STRL("SUPERGLOBAL_ARR"), 1), 1, &MyModule::rag_callback);
        return true;
    }

private:
    static zend_bool rag_callback(zend_string* name)
    {
        zval z;

        if (!std::strcmp(ZSTR_VAL(name), "SUPERGLOBAL_INT")) {
            ZVAL_LONG(&z, 42);
        }
        else if (!std::strcmp(ZSTR_VAL(name), "SUPERGLOBAL_ARR")) {
            array_init(&z);
        }
        else {
            EXPECT_FALSE(1);
            ZVAL_UNDEF(&z);
        }

        zend_hash_update(&EG(symbol_table), name, &z);
        return 0;
    }
};

}

TEST(SuperGlobals, testBasic)
{
    std::stringstream out;
    std::stringstream err;

    {
        MyModule ext("SuperGlobals", nullptr);
        TestSAPI sapi(out, err);
        sapi.addModule(ext);

        sapi.run([]() {
            zval r;
            ZVAL_UNDEF(&r);

            runPhpCode("$SUPERGLOBAL_INT;", r);
            ASSERT_EQ(IS_LONG, Z_TYPE(r));
            EXPECT_EQ(42,      zval_get_long(&r));

            runPhpCode("$SUPERGLOBAL_ARR;", r);
            ASSERT_EQ(IS_ARRAY, Z_TYPE(r));
            EXPECT_EQ(0,        zend_hash_num_elements(Z_ARRVAL(r)));
            EXPECT_EQ(2,        Z_REFCOUNT(r));

            zval_ptr_dtor(&r);
        });

        EXPECT_EQ("", out.str()); out.str(std::string());
        EXPECT_EQ("", err.str()); err.str(std::string());
    }

    {
        MyModule ext("SuperGlobals", nullptr);
        TestSAPI sapi(out, err);
        sapi.addModule(ext);

        sapi.run([]() {
            char sg_int[] = "SUPERGLOBAL_INT";
            char sg_arr[] = "SUPERGLOBAL_ARR";
            char sg_obj[] = "SUPERGLOBAL_OBJ";

            EXPECT_EQ(nullptr, zend_hash_str_find_ind(&EG(symbol_table), ZEND_STRL("SUPERGLOBAL_INT")));
            EXPECT_EQ(nullptr, zend_hash_str_find_ind(&EG(symbol_table), ZEND_STRL("SUPERGLOBAL_ARR")));

            EXPECT_EQ(1, zend_is_auto_global_str(sg_int, std::strlen(sg_int)));
            EXPECT_EQ(1, zend_is_auto_global_str(sg_arr, std::strlen(sg_arr)));
            EXPECT_EQ(0, zend_is_auto_global_str(sg_obj, std::strlen(sg_obj)));

            EXPECT_NE(nullptr, zend_hash_str_find_ind(&EG(symbol_table), ZEND_STRL("SUPERGLOBAL_INT")));
            EXPECT_NE(nullptr, zend_hash_str_find_ind(&EG(symbol_table), ZEND_STRL("SUPERGLOBAL_ARR")));
        });

        EXPECT_EQ("", out.str()); out.str(std::string());
        EXPECT_EQ("", err.str()); err.str(std::string());
    }

    {
        MyModule ext("SuperGlobals", nullptr);
        TestSAPI sapi(out, err);
        sapi.addModule(ext);

        sapi.run([]() {
            zval r;
            phpcxx::SuperGlobal sgint("SUPERGLOBAL_INT");
            phpcxx::SuperGlobal sgarr("SUPERGLOBAL_ARR");
            phpcxx::SuperGlobal sgobj("SUPERGLOBAL_OBJ");

            EXPECT_EQ(nullptr, zend_hash_str_find_ind(&EG(symbol_table), ZEND_STRL("SUPERGLOBAL_INT")));
            EXPECT_EQ(nullptr, zend_hash_str_find_ind(&EG(symbol_table), ZEND_STRL("SUPERGLOBAL_ARR")));

            phpcxx::Value& vi = sgint.get();
            phpcxx::Value& va = sgarr.get();

            EXPECT_THROW(sgobj.get(), std::runtime_error);

            EXPECT_EQ(phpcxx::Type::Integer,   sgint.type());
            EXPECT_EQ(42,                      vi.asLong());

            EXPECT_EQ(phpcxx::Type::Array,     sgarr.type());
            EXPECT_EQ(1,                       va.refCount());
            EXPECT_EQ(0,                       sgarr.size());

            EXPECT_EQ(phpcxx::Type::Undefined, sgobj.type());

            EXPECT_NE(nullptr, zend_hash_str_find_ind(&EG(symbol_table), ZEND_STRL("SUPERGLOBAL_INT")));
            EXPECT_NE(nullptr, zend_hash_str_find_ind(&EG(symbol_table), ZEND_STRL("SUPERGLOBAL_ARR")));

            vi = 43;
            EXPECT_EQ(43, sgint.get().asLong());

            runPhpCode("$SUPERGLOBAL_INT;", r);
            ASSERT_EQ(IS_LONG, Z_TYPE(r));
            EXPECT_EQ(43,      zval_get_long(&r));

            phpcxx::Array arr(va);
            EXPECT_EQ(2, va.refCount());
            arr[nullptr] = 24;
            EXPECT_EQ(0, sgarr.size());

            sgarr[nullptr] = 24;
            EXPECT_EQ(1, sgarr.size());
            runPhpCode("count($SUPERGLOBAL_ARR);", r);
            EXPECT_EQ(1, zval_get_long(&r));
            runPhpCode("$SUPERGLOBAL_ARR[0];", r);
            EXPECT_EQ(24, zval_get_long(&r));
        });

        EXPECT_EQ("", out.str()); out.str(std::string());
        EXPECT_EQ("", err.str()); err.str(std::string());
    }
}

TEST(SuperGlobals, testOperations)
{
    std::stringstream out;
    std::stringstream err;

    {
        MyModule ext("SuperGlobals", nullptr);
        TestSAPI sapi(out, err);
        sapi.addModule(ext);

        sapi.run([]() {
            phpcxx::ZendString za("a");
            phpcxx::string sa("a");
            phpcxx::Value va("a");
            phpcxx::Value v1(1);
            phpcxx::Value t(true);
            phpcxx::Value f(false);
            phpcxx::Value d0(0.0);
            phpcxx::Value n(nullptr);

            phpcxx::SuperGlobal sgint("SUPERGLOBAL_INT");
            phpcxx::SuperGlobal sgarr("SUPERGLOBAL_ARR");
            phpcxx::SuperGlobal sgobj("SUPERGLOBAL_OBJ");

            EXPECT_EQ(phpcxx::Type::Integer, sgint.type());
            EXPECT_EQ(phpcxx::Type::Array,   sgarr.type());

            EXPECT_THROW(sgint[nullptr] = 1, std::runtime_error);
            EXPECT_THROW(sgint[1]       = 1, std::runtime_error);
            EXPECT_THROW(sgint["a"]     = 1, std::runtime_error);
            EXPECT_THROW(sgint[sa]      = 1, std::runtime_error);
            EXPECT_THROW(sgint[za]      = 1, std::runtime_error);
            EXPECT_THROW(sgint[va]      = 1, std::runtime_error);

            EXPECT_THROW(sgobj[nullptr] = 1, std::runtime_error);
            EXPECT_THROW(sgobj[1]       = 1, std::runtime_error);
            EXPECT_THROW(sgobj["a"]     = 1, std::runtime_error);
            EXPECT_THROW(sgobj[sa]      = 1, std::runtime_error);
            EXPECT_THROW(sgobj[za]      = 1, std::runtime_error);
            EXPECT_THROW(sgobj[va]      = 1, std::runtime_error);

            EXPECT_FALSE(sgint.isset(1));
            EXPECT_FALSE(sgint.isset("a"));
            EXPECT_FALSE(sgint.isset(sa));
            EXPECT_FALSE(sgint.isset(za));
            EXPECT_FALSE(sgint.isset(va));
            EXPECT_FALSE(sgint.isset(v1));

            EXPECT_FALSE(sgobj.isset(1));
            EXPECT_FALSE(sgobj.isset("a"));
            EXPECT_FALSE(sgobj.isset(sa));
            EXPECT_FALSE(sgobj.isset(za));
            EXPECT_FALSE(sgobj.isset(va));
            EXPECT_FALSE(sgobj.isset(v1));

            EXPECT_NO_THROW(sgint.unset(1));
            EXPECT_NO_THROW(sgint.unset("a"));
            EXPECT_NO_THROW(sgint.unset(sa));
            EXPECT_NO_THROW(sgint.unset(za));
            EXPECT_NO_THROW(sgint.unset(va));
            EXPECT_NO_THROW(sgint.unset(v1));

            EXPECT_NO_THROW(sgobj.unset(1));
            EXPECT_NO_THROW(sgobj.unset("a"));
            EXPECT_NO_THROW(sgobj.unset(sa));
            EXPECT_NO_THROW(sgobj.unset(za));
            EXPECT_NO_THROW(sgobj.unset(va));
            EXPECT_NO_THROW(sgobj.unset(v1));

            EXPECT_NO_THROW(sgobj.unset(f));
            EXPECT_NO_THROW(sgobj.unset(t));
            EXPECT_NO_THROW(sgobj.unset(n));
            EXPECT_NO_THROW(sgobj.unset(d0));

            EXPECT_FALSE(sgarr.isset("0"));
            sgarr[nullptr] = 1;
            EXPECT_TRUE(sgarr.isset("0"));
            EXPECT_TRUE(sgarr.isset(f));
            EXPECT_TRUE(sgarr.isset(d0));
            EXPECT_EQ(1, sgarr["0"]);
            sgarr.unset("0");
            EXPECT_FALSE(sgarr.isset("0"));

            EXPECT_FALSE(sgarr.isset(1));
            sgarr[1] = 1;
            EXPECT_TRUE(sgarr.isset(1));
            EXPECT_TRUE(sgarr.isset(t));
            EXPECT_EQ(1, sgarr[1]);
            sgarr.unset(1);
            EXPECT_FALSE(sgarr.isset(1));

            EXPECT_FALSE(sgarr.isset("a"));
            sgarr["a"] = 1;
            EXPECT_TRUE(sgarr.isset("a"));
            EXPECT_EQ(1, sgarr["a"]);
            sgarr.unset("a");
            EXPECT_FALSE(sgarr.isset("a"));

            EXPECT_FALSE(sgarr.isset(sa));
            sgarr[sa] = 1;
            EXPECT_TRUE(sgarr.isset(sa));
            EXPECT_EQ(1, sgarr[sa]);
            sgarr.unset(sa);
            EXPECT_FALSE(sgarr.isset(sa));

            EXPECT_FALSE(sgarr.isset(za));
            sgarr[za] = 1;
            EXPECT_TRUE(sgarr.isset(za));
            EXPECT_EQ(1, sgarr[za]);
            sgarr.unset(za);
            EXPECT_FALSE(sgarr.isset(za));

            EXPECT_FALSE(sgarr.isset(va));
            sgarr[va] = 1;
            EXPECT_TRUE(sgarr.isset(va));
            EXPECT_EQ(1, sgarr[va]);
            sgarr.unset(va);
            EXPECT_FALSE(sgarr.isset(va));

            EXPECT_FALSE(sgarr.isset(d0));
            sgarr[d0] = 1;
            EXPECT_TRUE(sgarr.isset(d0));
            EXPECT_TRUE(sgarr.isset(f));
            EXPECT_TRUE(sgarr.isset(zend_long(0)));
            EXPECT_EQ(1, sgarr[d0]);
            EXPECT_EQ(1, sgarr["0"]);
            EXPECT_EQ(1, sgarr[f]);
            sgarr.unset(zend_long(0));
            EXPECT_FALSE(sgarr.isset(d0));

            EXPECT_FALSE(sgarr.isset(v1));
            sgarr[v1] = 1;
            EXPECT_TRUE(sgarr.isset(v1));
            EXPECT_TRUE(sgarr.isset(t));
            EXPECT_TRUE(sgarr.isset(1));
            EXPECT_EQ(1, sgarr[v1]);
            EXPECT_EQ(1, sgarr[1]);
            EXPECT_EQ(1, sgarr[t]);
            sgarr.unset(1);
            EXPECT_FALSE(sgarr.isset(v1));

            EXPECT_EQ(phpcxx::Type::Null, n.type());
            EXPECT_FALSE(sgarr.isset(n));
            sgarr[n] = 1;
            EXPECT_TRUE(sgarr.isset(n));
            EXPECT_FALSE(sgarr.isset(f));
            EXPECT_FALSE(sgarr.isset(d0));
            EXPECT_EQ(1, sgarr[n]);
            EXPECT_EQ(1, sgarr[""]);
        });

        EXPECT_EQ("", out.str()); out.str(std::string());
        EXPECT_EQ("", err.str()); err.str(std::string());
    }
}

TEST(SuperGlobals, testBuiltInSuperGlobals)
{
    std::stringstream out;
    std::stringstream err;

    TestSAPI sapi(out, err);

    sapi.run([]() {
        zval r;
        phpcxx::SuperGlobal _GET("_GET");

        runPhpCode("empty($_GET);", r);
        EXPECT_TRUE(zend_is_true(&r));
        EXPECT_EQ(0, _GET.size());
        EXPECT_EQ(0, phpcxx::SuperGlobal::orig_GET().size());

        _GET[1] = 1;
        runPhpCode("isset($_GET[1]);", r);
        EXPECT_TRUE(zend_is_true(&r));
        EXPECT_FALSE(phpcxx::SuperGlobal::orig_GET().isset(1));

        runPhpCode("$_GET[1];", r);
        EXPECT_EQ(1, zval_get_long(&r));

        runPhpCode("$_GET[2] = 2;");
        EXPECT_TRUE(_GET.isset(2));
        EXPECT_EQ(2, _GET[2].asLong());

        _GET.unset(1);
        runPhpCode("isset($_GET[1]);", r);
        EXPECT_FALSE(zend_is_true(&r));
        EXPECT_FALSE(_GET.isset(1));
    });

    EXPECT_EQ("", out.str()); out.str(std::string());
    EXPECT_EQ("", err.str()); err.str(std::string());

    sapi.run([]() {
        zval r;
        phpcxx::SuperGlobal GLOBALS("GLOBALS");

        runPhpCode("global $a, $b, $c; $a = 1; $b = 2; $c = 3;");
        EXPECT_TRUE(GLOBALS.isset("a"));
        EXPECT_TRUE(GLOBALS.isset("b"));
        EXPECT_TRUE(GLOBALS.isset("c"));
        EXPECT_EQ(1, GLOBALS["a"].asLong());
        EXPECT_EQ(2, GLOBALS["b"].asLong());
        EXPECT_EQ(3, GLOBALS["c"].asLong());

        GLOBALS["d"] = 4;
        GLOBALS["e"] = 5;
        EXPECT_TRUE(GLOBALS.isset("d"));
        EXPECT_TRUE(GLOBALS.isset("e"));
        EXPECT_EQ(4, GLOBALS["d"].asLong());
        EXPECT_EQ(5, GLOBALS["e"].asLong());

        runPhpCode("isset($GLOBALS['d']);", r);
        EXPECT_TRUE(zend_is_true(&r));
        runPhpCode("isset($GLOBALS['e']);", r);
        EXPECT_TRUE(zend_is_true(&r));
        runPhpCode("$GLOBALS['d'];", r);
        EXPECT_EQ(4, zval_get_long(&r));
        runPhpCode("$GLOBALS['e'];", r);
        EXPECT_EQ(5, zval_get_long(&r));

        runPhpCode("unset($GLOBALS['e']);");
        EXPECT_FALSE(GLOBALS.isset("e"));

        GLOBALS.unset("d");
        EXPECT_FALSE(GLOBALS.isset("d"));
        runPhpCode("isset($GLOBALS['d']);", r);
        EXPECT_FALSE(zend_is_true(&r));

        GLOBALS.unset("c");
        EXPECT_FALSE(GLOBALS.isset("c"));
        runPhpCode("isset($GLOBALS['c']);", r);
        EXPECT_FALSE(zend_is_true(&r));
    });

    EXPECT_EQ("", out.str()); out.str(std::string());
    EXPECT_EQ("", err.str()); err.str(std::string());
}

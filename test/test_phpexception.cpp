#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "phpcxx/module.h"
#include "phpcxx/fcall.h"
#include "phpcxx/phpexception.h"
#include "phpcxx/parameters.h"
#include "phpcxx/function.h"
#include "testsapi.h"
#include "globals.h"

namespace {

class MyModule : public phpcxx::Module {
public:
    using phpcxx::Module::Module;

protected:
    virtual std::vector<phpcxx::Function> functions() override
    {
        return {
            phpcxx::createFunction<&MyModule::test_simple_exception>("test_simple_exception"),
            phpcxx::createFunction<&MyModule::test_nested_exception>("test_nested_exception"),
            phpcxx::createFunction<&MyModule::throw_exception_vv>("throw_exception_23_vv"),
            phpcxx::createFunction<&MyModule::throw_exception_vn>("throw_exception_23_vn"),
            phpcxx::createFunction<&MyModule::throw_exception_nv>("throw_exception_23_nv"),
            phpcxx::createFunction<&MyModule::throw_exception_nn>("throw_exception_23_nn")
        };
    }

    static phpcxx::Value test_simple_exception()
    {
        bool exception_caught = false;
        try {
            phpcxx::call("throw_exception");
            EXPECT_FALSE(1);
        }
        catch (const phpcxx::PhpException& e) {
            exception_caught = true;

            EXPECT_EQ("LogicException",     e.className());
            EXPECT_EQ(nullptr,              e.previous());
            EXPECT_EQ("Exception message",  e.message());
            EXPECT_EQ(123,                  e.code());
            EXPECT_NE(phpcxx::string::npos, e.file().find("eval'd code"));
            EXPECT_EQ(4,                    e.line());
            EXPECT_STREQ(e.what(),          e.message().c_str());
        }
        catch (const std::exception& e) {
            EXPECT_FALSE(2);
        }

        EXPECT_EQ(nullptr, EG(exception));
        return exception_caught;
    }

    static phpcxx::Value test_nested_exception()
    {
        bool exception_caught = false;
        try {
            phpcxx::call("throw_nested_exception");
            EXPECT_FALSE(1);
        }
        catch (const phpcxx::PhpException& e) {
            exception_caught = true;

            EXPECT_EQ("Exception",          e.className());
            EXPECT_EQ("Top-level",          e.message());
            EXPECT_EQ(456,                  e.code());
            EXPECT_NE(phpcxx::string::npos, e.file().find("eval'd code"));
            EXPECT_EQ(8,                    e.line());
            EXPECT_STREQ(e.what(),          e.message().c_str());

            EXPECT_NE(nullptr, e.previous());
            const phpcxx::PhpException* p = e.previous();
            if (p != nullptr) {
                EXPECT_EQ("LogicException",     p->className());
                EXPECT_EQ(nullptr,              p->previous());
                EXPECT_EQ("Nested",             p->message());
                EXPECT_EQ(123,                  p->code());
                EXPECT_NE(phpcxx::string::npos, p->file().find("eval'd code"));
                EXPECT_EQ(5,                    p->line());
                EXPECT_STREQ(p->what(),         p->message().c_str());
            }

            phpcxx::Array trace;
            trace = e.trace();
            EXPECT_EQ(2, trace.size());
            EXPECT_EQ(phpcxx::Type::Array,      trace[0].type());
            EXPECT_EQ(phpcxx::Type::Array,      trace[1].type());
            EXPECT_EQ("throw_nested_exception", trace[0]["function"].toString());
            EXPECT_EQ("test_nested_exception",  trace[1]["function"].toString());

            try {
                throw;
            }
            catch (phpcxx::PhpException& f) {
                EXPECT_TRUE(EG(exception) != nullptr);

                EXPECT_EQ("Exception",          f.className());
                EXPECT_EQ("Top-level",          f.message());
                EXPECT_EQ(456,                  f.code());
                EXPECT_NE(phpcxx::string::npos, f.file().find("eval'd code"));
                EXPECT_EQ(8,                    f.line());
                EXPECT_STREQ(f.what(),          f.message().c_str());

                EXPECT_NE(nullptr, f.previous());
                const phpcxx::PhpException* p = f.previous();
                if (p != nullptr) {
                    EXPECT_EQ("LogicException",     p->className());
                    EXPECT_EQ(nullptr,              p->previous());
                    EXPECT_EQ("Nested",             p->message());
                    EXPECT_EQ(123,                  p->code());
                    EXPECT_NE(phpcxx::string::npos, p->file().find("eval'd code"));
                    EXPECT_EQ(5,                    p->line());
                    EXPECT_STREQ(p->what(),         p->message().c_str());
                }

                phpcxx::Array trace;
                trace = f.trace();
                EXPECT_EQ(2, trace.size());
                EXPECT_EQ(phpcxx::Type::Array,      trace[0].type());
                EXPECT_EQ(phpcxx::Type::Array,      trace[1].type());
                EXPECT_EQ("throw_nested_exception", trace[0]["function"].toString());
                EXPECT_EQ("test_nested_exception",  trace[1]["function"].toString());
            }

            // `throw;` rethrew the original object,
            // the destructor of the exception was NOT called
            EXPECT_FALSE(EG(exception) == nullptr);

            // Test move constructor
            try {
                throw;
            }
            catch (phpcxx::PhpException& z) { // catch by non-const reference so that we can move
                phpcxx::PhpException x(std::move(z));
                EXPECT_EQ("Exception",          x.className());
                EXPECT_EQ("Top-level",          x.message());
                EXPECT_EQ(456,                  x.code());
                EXPECT_NE(phpcxx::string::npos, x.file().find("eval'd code"));
                EXPECT_EQ(8,                    x.line());
            }
        }
        catch (const std::exception& e) {
            EXPECT_FALSE(2);
        }

        EXPECT_EQ(nullptr, EG(exception));
        return exception_caught;
    }

    static void throw_exception_nn()
    {
        EXPECT_TRUE(nullptr == EG(exception));
        zend_throw_exception(zend_exception_get_default(), "Exception", 0);
        EXPECT_TRUE(nullptr != EG(exception));
        throw phpcxx::PhpException();
    }

    static phpcxx::Value throw_exception_vn()
    {
        EXPECT_TRUE(nullptr == EG(exception));
        zend_throw_exception(zend_exception_get_default(), "Exception", 0);
        EXPECT_TRUE(nullptr != EG(exception));
        throw phpcxx::PhpException();
    }

    static void throw_exception_nv(phpcxx::Parameters&)
    {
        EXPECT_TRUE(nullptr == EG(exception));
        zend_throw_exception(zend_exception_get_default(), "Exception", 0);
        EXPECT_TRUE(nullptr != EG(exception));
        throw phpcxx::PhpException();
    }

    static phpcxx::Value throw_exception_vv(phpcxx::Parameters&)
    {
        EXPECT_TRUE(nullptr == EG(exception));
        zend_throw_exception(zend_exception_get_default(), "Exception", 0);
        EXPECT_TRUE(nullptr != EG(exception));
        throw phpcxx::PhpException();
    }
};

}

TEST(PhpException, TestSimpleException)
{
    MyModule module("PhpException", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([&err] {
            runPhpCode(R"___(
function throw_exception()
{
    throw new LogicException("Exception message", 123);
}
)___"
            );

            phpcxx::Value res = phpcxx::call("test_simple_exception");
            EXPECT_EQ(phpcxx::Type::True, res.type());
            err << "SUCCESS" << std::flush;
        });
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("SUCCESS", e);
}

TEST(PhpException, TestNestedException)
{
    MyModule module("PhpException", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([&err] {
            runPhpCode(R"___(
function throw_nested_exception()
{
    try {
        throw new LogicException("Nested", 123);
    }
    catch (Exception $e) {
        throw new Exception("Top-level", 456, $e);
    }
}
)___"
            );

            phpcxx::Value res = phpcxx::call("test_nested_exception");
            EXPECT_EQ(phpcxx::Type::True, res.type());
            err << "SUCCESS" << std::flush;
        });
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("SUCCESS", e);
}

TEST(PhpException, TestInvalidArguments)
{
    MyModule module("PhpException", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([&err] {
            ASSERT_EQ(nullptr, EG(exception));
            ASSERT_THROW(phpcxx::PhpException(), std::logic_error);
            err << "SUCCESS" << std::flush;
        });
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("SUCCESS", e);
}

/**
 * @see https://github.com/sjinks/php-cxx/issues/23
 */
TEST(PhpException, TestIssue23)
{
    std::stringstream out;
    std::stringstream err;
    std::string o;
    std::string e;

    bool success = false;

    {
        MyModule module("PhpException", "0.0");
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([&success] {
            DummyStackFrame dummy;
            try {
                phpcxx::call("throw_exception_23_vv");
                ADD_FAILURE();
            }
            catch (phpcxx::PhpException& e) {
                success = true;
            }
            catch (const std::exception& e) {
                ADD_FAILURE();
            }
        });

        EXPECT_TRUE(success);
    }

    o = out.str(); out.str("");
    e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);

    {
        MyModule module("PhpException", "0.0");
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([&success] {
            DummyStackFrame dummy;
            try {
                phpcxx::call("throw_exception_23_vn");
                ADD_FAILURE();
            }
            catch (phpcxx::PhpException& e) {
                success = true;
            }
            catch (const std::exception& e) {
                ADD_FAILURE();
            }
        });

        EXPECT_TRUE(success);
    }

    o = out.str(); out.str("");
    e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);

    {
        MyModule module("PhpException", "0.0");
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([&success] {
            DummyStackFrame dummy;
            try {
                phpcxx::call("throw_exception_23_nn");
                ADD_FAILURE();
            }
            catch (phpcxx::PhpException& e) {
                success = true;
            }
            catch (const std::exception& e) {
                ADD_FAILURE();
            }
        });

        EXPECT_TRUE(success);
    }

    o = out.str(); out.str("");
    e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);

    {
        MyModule module("PhpException", "0.0");
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([&success] {
            DummyStackFrame dummy;
            try {
                phpcxx::call("throw_exception_23_nv");
                ADD_FAILURE();
            }
            catch (phpcxx::PhpException& e) {
                success = true;
            }
            catch (const std::exception& e) {
                ADD_FAILURE();
            }
        });

        EXPECT_TRUE(success);
    }

    o = out.str(); out.str("");
    e = err.str(); err.str("");
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);
}

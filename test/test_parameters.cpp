#include <initializer_list>
#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "phpcxx/module.h"
#include "phpcxx/function.h"
#include "phpcxx/fcall.h"
#include "phpcxx/types.h"
#include "phpcxx/vector.h"
#include "testsapi.h"
#include "globals.h"

extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_exceptions.h>
}

namespace {

class MyModule : public phpcxx::Module {
public:
    using phpcxx::Module::Module;

protected:
    virtual std::vector<phpcxx::Function> functions() override
    {
        return {
            phpcxx::createFunction<&MyModule::test_parameters>(
                "test_parameters",
                9,
                {
                    phpcxx::byValue("a", phpcxx::ArgumentType::Any, false),
                    phpcxx::byValue("b", phpcxx::ArgumentType::Bool, false),
                    phpcxx::byValue("c", phpcxx::ArgumentType::Integer, false),
                    phpcxx::byValue("d", phpcxx::ArgumentType::Double, false),
                    phpcxx::byValue("e", phpcxx::ArgumentType::String, false),
                    phpcxx::byValue("f", phpcxx::ArgumentType::Array, false),
                    phpcxx::byValue("g", phpcxx::ArgumentType::Object, false),
                    phpcxx::byValue("h", phpcxx::ArgumentType::Resource, false),
                    phpcxx::byValue("i", phpcxx::ArgumentType::Callable, false)
                }
            )
        };
    }

private:
    static void test_parameters(phpcxx::Parameters& p)
    {
        if (!p.verify()) {
            return;
        }

        ASSERT_EQ(9, p.size());
        EXPECT_TRUE(phpcxx::Type::True == p[1].type() || phpcxx::Type::False == p[1].type());
        EXPECT_EQ(phpcxx::Type::Integer,  p[2].type());
        EXPECT_EQ(phpcxx::Type::Double,   p[3].type());
        EXPECT_EQ(phpcxx::Type::String,   p[4].type());
        EXPECT_EQ(phpcxx::Type::Array,    p[5].type());
        EXPECT_EQ(phpcxx::Type::Object,   p[6].type());
        EXPECT_EQ(phpcxx::Type::Resource, p[7].type());
        EXPECT_TRUE(phpcxx::Type::String == p[8].type() || phpcxx::Type::Array == p[8].type() || phpcxx::Type::Object == p[8].type());
    }
};

}

TEST(Parameters, TestCoercion)
{
    MyModule module("Parameters", "0.0");

    std::stringstream out;
    std::stringstream err;
    std::string o;
    std::string e;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        sapi.run([]() {
            runPhpCode(R"___(
$a = 0;
$b = true;
$c = 1;
$d = 4.0;
$e = "";
$f = array();
$g = new stdClass();
$h = tmpfile();
$i = 'sort';

test_parameters($a, $b, $c, $d, $e, $f, $g, $h, $i);

$d = 4;
test_parameters($a, $b, $c, $d, $e, $f, $g, $h, $i);

$d = "4";
test_parameters($a, $b, $c, $d, $e, $f, $g, $h, $i);

$b = 0;
test_parameters($a, $b, $c, $d, $e, $f, $g, $h, $i);

$b = 2;
test_parameters($a, $b, $c, $d, $e, $f, $g, $h, $i);

$c = false;
test_parameters($a, $b, $c, $d, $e, $f, $g, $h, $i);

$e = false;
test_parameters($a, $b, $c, $d, $e, $f, $g, $h, $i);

$b = "aaa";
test_parameters($a, $b, $c, $d, $e, $f, $g, $h, $i);
)___");
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("", o);
        EXPECT_EQ("", e);
    }
}

TEST(Parameters, TestVerify)
{
    std::stringstream out;
    std::stringstream err;
    std::string o;
    std::string e;

    {
        MyModule module("Parameters", "0.0");
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        sapi.run([&err]() {
            DummyStackFrame dummy;

            try {
                phpcxx::call("test_parameters", 0, 0, 0, 0, 0, 0, 0, 0, 0);
                ADD_FAILURE();
            }
            catch (const phpcxx::PhpException& ex) {
                EXPECT_STREQ("Argument 6 passed to test_parameters() must be of the type array, integer given", ex.what());
                err << ex.what() << std::endl;
            }
            catch (const std::exception& e) {
                ADD_FAILURE();
            }

            try {
                runPhpCode(R"___(
function exception_error_handler($severity, $message, $file, $line)
{
    throw new ErrorException($message, 0, $severity, $file, $line);
}

set_error_handler("exception_error_handler");
)___");
                phpcxx::call("test_parameters", 0);
                ADD_FAILURE();
            }
            catch (const phpcxx::PhpException& ex) {
                EXPECT_STREQ("test_parameters() expects exactly 9 parameters, 1 given", ex.what());
                err << ex.what() << std::endl;
            }
            catch (const std::exception& e) {
                ADD_FAILURE();
            }
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("", o);
        EXPECT_EQ(
            "Argument 6 passed to test_parameters() must be of the type array, integer given\n"
            "test_parameters() expects exactly 9 parameters, 1 given\n"
            ,
            e
        );
    }

    {
        MyModule module("Parameters", "0.0");
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        sapi.run([]() {
            DummyStackFrame dummy;
            phpcxx::call("test_parameters", 0);
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("", o);
        EXPECT_NE(std::string::npos, e.find("test_parameters() expects exactly 9 parameters, 1 given"));
    }
}

TEST(Parameters, TestInitialization)
{
    std::stringstream out;
    std::stringstream err;
    std::string o;
    std::string e;

    {
        MyModule module("Parameters", "0.0");
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        sapi.run([]() {
            DummyStackFrame dummy;

            phpcxx::Value v1 = true;
            phpcxx::Value v2 = false;
            phpcxx::Value v3 = nullptr;

            phpcxx::vector<phpcxx::Value*> v{ &v1, &v2, &v3 };
            phpcxx::Parameters p1(v);
            phpcxx::Parameters p2({ &v1, &v2, &v3 });

            ASSERT_EQ(p1.size(), p2.size());
            ASSERT_EQ(3, p1.size());
            EXPECT_EQ(v1, p1[0]);
            EXPECT_EQ(v2, p1[1]);
            EXPECT_EQ(v3, p1[2]);
            EXPECT_EQ(v1, p2[0]);
            EXPECT_EQ(v2, p2[1]);
            EXPECT_EQ(v3, p2[2]);
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("", o);
        EXPECT_EQ("", e);
    }
}

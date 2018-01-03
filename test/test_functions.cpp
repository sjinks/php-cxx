#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include "phpcxx/module.h"
#include "phpcxx/function.h"
#include "phpcxx/fcall.h"
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

    static void swap(phpcxx::Parameters& p)
    {
        phpcxx::Value tmp = p[0];
        p[0] = p[1];
        p[1] = tmp;
    }

protected:
    virtual std::vector<phpcxx::Function> functions() override
    {
        return {
            phpcxx::createFunction<&MyModule::func1>("func1"),
            phpcxx::createFunction<&MyModule::func2>(
                "func2",
                1,
                {
                    phpcxx::byValue("a", phpcxx::ArgumentType::Any, false),
                    phpcxx::byReference("b", "stdClass", true)
                }
            ),
            phpcxx::createFunction<&MyModule::func3>("func3")
                .setTypeHint(phpcxx::ArgumentType::Integer, false)
                .addArgument(
                    phpcxx::Argument("a")
                        .setType(phpcxx::ArgumentType::Array, true)
                )
            ,
            phpcxx::createFunction<&MyModule::func4>("func4")
                .setTypeHint(phpcxx::ArgumentType::Any, false)
                .addArgument(
                    phpcxx::Argument("a")
                        .setType(phpcxx::ArgumentType::Any, true)
                ),
            phpcxx::createFunction<&MyModule::func5>(
                "func5",
                1,
                {
                    phpcxx::byValue("a", phpcxx::ArgumentType::Any, false),
                    phpcxx::byValue("b", "stdClass", true)
                }
            ),
            phpcxx::createFunction<&MyModule::func5>(
                "func6",
                2,
                {
                    phpcxx::byValue("a", phpcxx::ArgumentType::Any, false),
                    phpcxx::byReference("b", "stdClass", true)
                }
            ),
            phpcxx::createFunction<&MyModule::func5>(
                "func7",
                2,
                {
                    phpcxx::byValue("a", phpcxx::ArgumentType::Any, false),
                    phpcxx::byValue("b", "stdClass", true)
                }
            ),
            phpcxx::createFunction<&MyModule::swap>("swap")
                .setNumberOfRequiredArguments(2)
                .addArgument(phpcxx::byReference("a"))
                .addArgument(phpcxx::byReference("b")),
            phpcxx::createFunction<&MyModule::func_throwing>("func_throwing"),
            phpcxx::createFunction<&MyModule::call_func_throwing>("call_func_throwing"),
            phpcxx::createFunction<&MyModule::func_throwing3>("func_throwing3"),
            phpcxx::createFunction<&MyModule::call_func_throwing3>("call_func_throwing3"),
            phpcxx::createFunction<&MyModule::func_bailingout>("func_bailingout"),
            phpcxx::createFunction<&MyModule::call_func_bailingout>("call_func_bailingout")
        };
    }

private:
    static void func1()
    {
        php_printf("%s\n", "func1");
    }

    static void func2(phpcxx::Parameters& p)
    {
        if (p.size() > 0) {
            phpcxx::Value a = p[0];
            php_printf("func2: a = %s\n", a.toString().c_str());
        }
    }

    static phpcxx::Value func3()
    {
        return 42;
    }

    static phpcxx::Value func4(phpcxx::Parameters& p)
    {
        return p.size() ? p[0] : phpcxx::Value();
    }

    static void func5(phpcxx::Parameters& p)
    {
    }

    static void func_throwing()
    {
        EXPECT_TRUE(EG(exception) == nullptr);
        php_printf("> %s\n", "func_throwing");
        zend_throw_exception(zend_exception_get_default(), "Message", 1);
        EXPECT_TRUE(EG(exception) != nullptr);
        php_printf("< %s\n", "func_throwing");
    }

    static void call_func_throwing()
    {
        EXPECT_THROW(phpcxx::call("func_throwing"), phpcxx::PhpException);

        bool exception_caught = false;
        try {
            phpcxx::call("func_throwing");
            EXPECT_TRUE(0);
        }
        catch (const phpcxx::PhpException& e) {
            EXPECT_EQ(1, e.code());
            EXPECT_EQ("Message", e.message());
            EXPECT_EQ("Exception", e.className());
            exception_caught = true;
        }

        EXPECT_TRUE(EG(exception) == nullptr);
        EXPECT_TRUE(exception_caught);
    }

    static void func_throwing3()
    {
        php_printf("+ %s\n", "func_throwing3");
        throw std::runtime_error("crash boom bang");
    }

    static void call_func_throwing3()
    {
        EXPECT_THROW(phpcxx::call("func_throwing3"), phpcxx::PhpException);

        bool exception_caught = false;
        try {
            phpcxx::call("func_throwing3");
            // Should not happen
            EXPECT_TRUE(0);
        }
        catch (phpcxx::PhpException& e) {
            EXPECT_EQ(0, e.code());
            EXPECT_EQ("Unhandled C++ exception: crash boom bang", e.message());
            exception_caught = true;
        }

        EXPECT_TRUE(EG(exception) == nullptr);
        EXPECT_TRUE(exception_caught);
    }

    static void func_bailingout()
    {
        php_printf("+ %s\n", "func_bailingout");
        zend_error(E_ERROR, "Bailing out");
    }

    static void call_func_bailingout()
    {
        bool bailed_out = false;
        zend_try {
            phpcxx::call("func_bailingout");
            // Should not happen
            EXPECT_TRUE(0);
        }
        zend_catch {
            bailed_out = 1;
            EXPECT_TRUE(1);
        }
        zend_end_try()

        EXPECT_TRUE(bailed_out);
    }
};

}

TEST(FunctionsTest, Definitions)
{
    MyModule module("Functions", "0.0");

    std::stringstream out;
    std::stringstream err;
    std::string o;
    std::string e;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        sapi.run([&out, &err, &o, &e]() {
            zval res;
            ZVAL_UNDEF(&res);

            runPhpCode("function_exists('func1');", res);
            EXPECT_EQ(IS_TRUE, Z_TYPE(res));
            runPhpCode("function_exists('func2');", res);
            EXPECT_EQ(IS_TRUE, Z_TYPE(res));
            runPhpCode("function_exists('func3');", res);
            EXPECT_EQ(IS_TRUE, Z_TYPE(res));
            runPhpCode("function_exists('func4');", res);
            EXPECT_EQ(IS_TRUE, Z_TYPE(res));

            // void func1(void)
            runPhpCode(
                "$r = new ReflectionFunction('func1'); "
                "echo $r->getName(), PHP_EOL; "
                "echo $r->getNumberOfParameters(), PHP_EOL; "
                "echo $r->getNumberOfRequiredParameters(), PHP_EOL; "
                "echo (int)$r->hasReturnType(), PHP_EOL; "
            );

            o = out.str(); out.str(std::string());
            e = err.str(); err.str(std::string());
            EXPECT_EQ("func1\n0\n0\n0\n", o);
            EXPECT_EQ("", e);

            // void func2($a, stdClass& $b = null)
            runPhpCode(
                "$r = new ReflectionFunction('func2'); "
                "echo $r->getName(), PHP_EOL; "
                "echo $r->getNumberOfParameters(), PHP_EOL; "
                "echo $r->getNumberOfRequiredParameters(), PHP_EOL; "
                "echo (int)$r->hasReturnType(), PHP_EOL; "
                "echo $r->getReturnType(), PHP_EOL; "
                "$params = $r->getParameters(); "
                "echo $params[0]->getName(), ' ', $params[0]->getType(), ' ', (int)$params[0]->allowsNull(), ' ', (int)$params[0]->canBePassedByValue(), PHP_EOL;"
                "echo $params[1]->getName(), ' ', $params[1]->getType(), ' ', (int)$params[1]->allowsNull(), ' ', (int)$params[1]->canBePassedByValue(), PHP_EOL;"
            );

            o = out.str(); out.str(std::string());
            e = err.str(); err.str(std::string());
            EXPECT_EQ("func2\n2\n1\n0\n\na  0 1\nb stdClass 1 0\n", o);
            EXPECT_EQ("", e);

            // int func3(array $a = null)
            runPhpCode(
                "$r = new ReflectionFunction('func3'); "
                "echo $r->getName(), PHP_EOL; "
                "echo $r->getNumberOfParameters(), PHP_EOL; "
                "echo $r->getNumberOfRequiredParameters(), PHP_EOL; "
                "echo (int)$r->hasReturnType(), PHP_EOL; "
                "echo $r->getReturnType(), PHP_EOL; "
                "$params = $r->getParameters(); "
                "echo $params[0]->getName(), ' ', $params[0]->getType(), ' ', (int)$params[0]->allowsNull(), ' ', (int)$params[0]->canBePassedByValue(), PHP_EOL;"
            );

            o = out.str(); out.str(std::string());
            e = err.str(); err.str(std::string());
            EXPECT_EQ("func3\n1\n0\n1\nint\na array 1 1\n", o);
            EXPECT_EQ("", e);

            // func4($a = null)
            // return type Any = no type
            runPhpCode(
                "$r = new ReflectionFunction('func4'); "
                "echo $r->getName(), PHP_EOL; "
                "echo $r->getNumberOfParameters(), PHP_EOL; "
                "echo $r->getNumberOfRequiredParameters(), PHP_EOL; "
                "echo (int)$r->hasReturnType(), PHP_EOL; "
                "echo $r->getReturnType(), PHP_EOL; "
                "$params = $r->getParameters(); "
                "echo $params[0]->getName(), ' ', $params[0]->getType(), ' ', (int)$params[0]->allowsNull(), ' ', (int)$params[0]->canBePassedByValue(), PHP_EOL;"
            );

            o = out.str(); out.str(std::string());
            e = err.str(); err.str(std::string());
            EXPECT_EQ("func4\n1\n0\n0\n\na  1 1\n", o);
            EXPECT_EQ("", e);

            // void func5($a, stdClass $b = null)
            runPhpCode(
                "$r = new ReflectionFunction('func5'); "
                "echo $r->getName(), PHP_EOL; "
                "echo $r->getNumberOfParameters(), PHP_EOL; "
                "echo $r->getNumberOfRequiredParameters(), PHP_EOL; "
                "echo (int)$r->hasReturnType(), PHP_EOL; "
                "echo $r->getReturnType(), PHP_EOL; "
                "$params = $r->getParameters(); "
                "echo $params[0]->getName(), ' ', $params[0]->getType(), ' ', (int)$params[0]->allowsNull(), ' ', (int)$params[0]->canBePassedByValue(), PHP_EOL;"
                "echo $params[1]->getName(), ' ', $params[1]->getType(), ' ', (int)$params[1]->allowsNull(), ' ', (int)$params[1]->canBePassedByValue(), PHP_EOL;"
            );

            o = out.str(); out.str(std::string());
            e = err.str(); err.str(std::string());
            EXPECT_EQ("func5\n2\n1\n0\n\na  0 1\nb stdClass 1 1\n", o);
            EXPECT_EQ("", e);

            // void func6($a, stdClass& $b = null), $b is NOT optional
            runPhpCode(
                "$r = new ReflectionFunction('func6'); "
                "echo $r->getName(), PHP_EOL; "
                "echo $r->getNumberOfParameters(), PHP_EOL; "
                "echo $r->getNumberOfRequiredParameters(), PHP_EOL; "
                "echo (int)$r->hasReturnType(), PHP_EOL; "
                "echo $r->getReturnType(), PHP_EOL; "
                "$params = $r->getParameters(); "
                "echo $params[0]->getName(), ' ', $params[0]->getType(), ' ', (int)$params[0]->allowsNull(), ' ', (int)$params[0]->canBePassedByValue(), PHP_EOL;"
                "echo $params[1]->getName(), ' ', $params[1]->getType(), ' ', (int)$params[1]->allowsNull(), ' ', (int)$params[1]->canBePassedByValue(), PHP_EOL;"
            );

            o = out.str(); out.str(std::string());
            e = err.str(); err.str(std::string());
            EXPECT_EQ("func6\n2\n2\n0\n\na  0 1\nb stdClass 1 0\n", o);
            EXPECT_EQ("", e);

            // void func7($a, stdClass $b = null), $b is NOT optional
            runPhpCode(
                "$r = new ReflectionFunction('func7'); "
                "echo $r->getName(), PHP_EOL; "
                "echo $r->getNumberOfParameters(), PHP_EOL; "
                "echo $r->getNumberOfRequiredParameters(), PHP_EOL; "
                "echo (int)$r->hasReturnType(), PHP_EOL; "
                "echo $r->getReturnType(), PHP_EOL; "
                "$params = $r->getParameters(); "
                "echo $params[0]->getName(), ' ', $params[0]->getType(), ' ', (int)$params[0]->allowsNull(), ' ', (int)$params[0]->canBePassedByValue(), PHP_EOL;"
                "echo $params[1]->getName(), ' ', $params[1]->getType(), ' ', (int)$params[1]->allowsNull(), ' ', (int)$params[1]->canBePassedByValue(), PHP_EOL;"
            );

            o = out.str(); out.str(std::string());
            e = err.str(); err.str(std::string());
            EXPECT_EQ("func7\n2\n2\n0\n\na  0 1\nb stdClass 1 1\n", o);
            EXPECT_EQ("", e);

            // Make sure PHP accepts our `null` values
            runPhpCode(
                "$x = null; "
                "func5(null, null); "
                "func6(null, $x); "
                "func7(null, null); "
            );

            o = out.str(); out.str(std::string());
            e = err.str(); err.str(std::string());
            EXPECT_EQ("", o);
            EXPECT_EQ("", e);

            // swap(&$a, &$b)
            // return type Any = no type
            runPhpCode(
                "$r = new ReflectionFunction('swap'); "
                "echo $r->getName(), PHP_EOL; "
                "echo $r->getNumberOfParameters(), PHP_EOL; "
                "echo $r->getNumberOfRequiredParameters(), PHP_EOL; "
                "echo (int)$r->hasReturnType(), PHP_EOL; "
                "echo $r->getReturnType(), PHP_EOL; "
                "$params = $r->getParameters(); "
                "echo $params[0]->getName(), ' ', $params[0]->getType(), ' ', (int)$params[0]->allowsNull(), ' ', (int)$params[0]->canBePassedByValue(), PHP_EOL;"
                "echo $params[1]->getName(), ' ', $params[1]->getType(), ' ', (int)$params[1]->allowsNull(), ' ', (int)$params[1]->canBePassedByValue(), PHP_EOL;"
            );

            o = out.str(); out.str(std::string());
            e = err.str(); err.str(std::string());
            EXPECT_EQ("swap\n2\n2\n0\n\na  1 0\nb  1 0\n", o);
            EXPECT_EQ("", e);
        });
    }

    o = out.str(); out.str(std::string());
    e = err.str(); err.str(std::string());
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);
}

TEST(FunctionsTest, SimpleCalls)
{
    MyModule module("Functions", "0.0");

    std::stringstream out;
    std::stringstream err;
    std::string o;
    std::string e;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        std::string o;
        std::string e;

        sapi.run([]() {
            phpcxx::call("func1");
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("func1\n", o);
        EXPECT_EQ("", e);

        sapi.run([]() {
            phpcxx::call("func2", 44);
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("func2: a = 44\n", o);
        EXPECT_EQ("", e);

        sapi.run([]() {
            /* Dirty hack: if we invoke call_func_throwing() directly
             * without runPhpCode(), ZE will bail out.
             * It looks like it bails out somewhere in zend_call_function()
             * because the function handler runs, and the code in
             * phpcxx::call() does not, and the control is transferred to
             * zend_catch block of TestSAPI::run()
             */
            runPhpCode("call_func_throwing();");
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("> func_throwing\n< func_throwing\n> func_throwing\n< func_throwing\n", o);
        EXPECT_EQ("", e);

        sapi.run([]() {
            runPhpCode("call_func_throwing3();");
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("+ func_throwing3\n+ func_throwing3\n", o);
        EXPECT_EQ("", e);

        sapi.run([]() {
            runPhpCode("call_func_bailingout();");
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("+ func_bailingout\n", o);
        EXPECT_NE(std::string::npos, e.find("Bailing out"));
        EXPECT_NE(std::string::npos, e.find("Fatal error"));

        out << "Done";
    }

    o = out.str(); out.str(std::string());
    e = err.str(); err.str(std::string());
    EXPECT_EQ("Done", o);
    EXPECT_EQ("", e);
}

TEST(FunctionsTest, ParamsByReference)
{
    MyModule module("Functions", "0.0");

    std::stringstream out;
    std::stringstream err;
    std::string o;
    std::string e;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);

        std::string o;
        std::string e;

        sapi.run([&out, &err, &o, &e]() {
            phpcxx::Value a("This is a");
            phpcxx::Value b("This is b");

            a.reference();
            b.reference();
            phpcxx::call("swap", a, b);
            EXPECT_EQ("This is a", b.toString());
            EXPECT_EQ("This is b", a.toString());
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("", o);
        EXPECT_EQ("", e);

        sapi.run([&out, &err, &o, &e]() {
            runPhpCode(
                "$a = 'This is a'; "
                "$b = 'This is b'; "
                "swap($a, $b); "
                "echo $a, PHP_EOL, $b, PHP_EOL;"
            );
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("This is b\nThis is a\n", o);
        EXPECT_EQ("", e);

        sapi.run([&out, &err, &o, &e]() {
            phpcxx::Value a("This is a");
            phpcxx::Value b("This is b");

            a.reference();
            b.reference();
            phpcxx::vector<phpcxx::Value*> v({ &a, &b });
            phpcxx::Parameters params(v);
            MyModule::swap(params);
            EXPECT_EQ("This is a", b.toString());
            EXPECT_EQ("This is b", a.toString());
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("", o);
        EXPECT_EQ("", e);
    }

    o = out.str(); out.str(std::string());
    e = err.str(); err.str(std::string());
    EXPECT_EQ("", o);
    EXPECT_EQ("", e);
}

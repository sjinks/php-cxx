#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <Zend/zend.h>
#include "phpcxx/module.h"
#include "phpcxx/function.h"
#include "phpcxx/call.h"
#include "testsapi.h"
#include "globals.h"

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
                { phpcxx::byValue("a", phpcxx::ArgumentType::Any, false) },
                { phpcxx::byReference("b", "stdClass", true) }
            ),
            phpcxx::createFunction<&MyModule::func3>("func3")
                .setTypeHint(phpcxx::ArgumentType::Integer)
                .setAllowNull(false)
                .addOptionalArgument(
                    phpcxx::Argument("a")
                        .setType(phpcxx::ArgumentType::Array)
                        .setNullable(true)
                )
            ,
            phpcxx::createFunction<&MyModule::func4>("func4")
                .setTypeHint(phpcxx::ArgumentType::Any)
                .setAllowNull(false)
                .addOptionalArgument(
                    phpcxx::Argument("a")
                        .setType(phpcxx::ArgumentType::Any)
                        .setNullable(true)
                ),
            phpcxx::createFunction<&MyModule::swap>("swap")
                .addRequiredArgument(phpcxx::byReference("a"))
                .addRequiredArgument(phpcxx::byReference("b"))
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

    static void func5(zend_execute_data* execute_data, zval* return_value)
    {

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
        sapi.initialize();

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
        sapi.initialize();

        std::string o;
        std::string e;

        sapi.run([&out, &err, &o, &e]() {
            phpcxx::call("func1");
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("func1\n", o);
        EXPECT_EQ("", e);

        sapi.run([&out, &err, &o, &e]() {
            phpcxx::call("func2", 44);
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("func2: a = 44\n", o);
        EXPECT_EQ("", e);
    }

    o = out.str(); out.str(std::string());
    e = err.str(); err.str(std::string());
    EXPECT_EQ("", o);
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
        sapi.initialize();

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

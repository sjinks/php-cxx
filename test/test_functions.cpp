#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <Zend/zend.h>
#include "phpcxx/module.h"
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
                )
        };
    }

private:
    static void func1()
    {
    }

    static void func2(phpcxx::Parameters& p)
    {
    }

    static phpcxx::Value func3()
    {
        return 42;
    }

    static phpcxx::Value func4(phpcxx::Parameters& p)
    {
        return p.count() ? p[0] : phpcxx::Value();
    }
};

}


TEST(FunctionsTest, TestDefinitions)
{
    MyModule module("Functions", "0.0");

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        std::string o;
        std::string e;

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
        });

        o = out.str(); out.str(std::string());
        e = err.str(); err.str(std::string());
        EXPECT_EQ("", o);
        EXPECT_EQ("", e);
    }
}

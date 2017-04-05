#include <sstream>
#include <string>
#include <gtest/gtest.h>
#include "phpcxx/module.h"
#include "testsapi.h"

extern "C" {
#include <main/SAPI.h>
}

namespace {

class MyModuleGlobals : public phpcxx::ModuleGlobals {
public:
    MyModuleGlobals() : m_s("the fields are white") {}

    virtual ~MyModuleGlobals() override
    {
        sapi_module.ub_write(ZEND_STRL("~MyModuleGlobals\n"));
    }

    const std::string& s() const { return this->m_s; }
    void setS(const std::string& v) { this->m_s = v; }

private:
    std::string m_s;
};

class MyModule : public phpcxx::Module {
public:
    MyModule(const char* name, const char* version)
        : phpcxx::Module(name, version)
    {
        this->setGlobalsConstructor(MyModule::globals_ctor);
        this->setGlobalsDestructor(MyModule::globals_dtor);
    }

protected:
    static void globals_ctor(zend_phpcxx_globals* g)
    {
        sapi_module.ub_write(ZEND_STRL("globalsConstructor\n"));
        g->globals = new MyModuleGlobals();
    }

    static void globals_dtor(zend_phpcxx_globals* g)
    {
        sapi_module.ub_write(ZEND_STRL("globalsDestructor\n"));
        delete g->globals;
    }

    virtual bool moduleStartup() override
    {
        sapi_module.ub_write(ZEND_STRL("moduleStartup\n"));
        php_output_flush();
        return true;
    }
};

}

TEST(ModuleGlobalsTest, TestModuleGlobals)
{
    MyModule module("ModuleGlobals", "0.0");
    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(module);
        sapi.initialize();

        std::string o = out.str(); out.str("");
        std::string e = err.str(); err.str("");
        EXPECT_EQ("", e);
        EXPECT_EQ("globalsConstructor\nmoduleStartup\n", o);

        sapi.run([&module]() {
            MyModuleGlobals* g1 = static_cast<MyModuleGlobals*>(module.globals());
            MyModuleGlobals* g2 = dynamic_cast<MyModuleGlobals*>(module.globals());
            ASSERT_TRUE(g1 != nullptr);
            ASSERT_TRUE(g2 != nullptr);
            ASSERT_EQ(g1, g2);

            EXPECT_EQ("the fields are white", g1->s());
            std::string s("the fields are white with harvest in the morning light");
            g1->setS(s);
            EXPECT_EQ(s, (static_cast<MyModuleGlobals*>(module.globals()))->s());
        });
    }

    std::string o = out.str(); out.str("");
    std::string e = err.str(); err.str("");
    EXPECT_EQ("", e);
    EXPECT_EQ("globalsDestructor\n~MyModuleGlobals\n", o);
}


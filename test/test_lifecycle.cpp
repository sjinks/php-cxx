#include <memory>
#include <iostream>
#include <sstream>
#include <gtest/gtest.h>
#include <Zend/zend.h>
#include <Zend/zend_API.h>
#include "phpcxx/module.h"
#include "testsapi.h"

namespace {

class MyModule : public phpcxx::Module {
public:
    int module_startup_called   = 0;
    int module_shutdown_called  = 0;
    int request_startup_called  = 0;
    int request_shutdown_called = 0;

    using phpcxx::Module::Module;

protected:
    virtual bool moduleStartup() override
    {
        ++this->module_startup_called;
        return true;
    }

    virtual bool moduleShutdown() override
    {
        ++this->module_shutdown_called;
        return true;
    }

    virtual bool requestStartup() override
    {
        ++this->request_startup_called;
        return true;
    }

    virtual bool requestShutdown() override
    {
        ++this->request_shutdown_called;
        return true;
    }
};

class LoadOtherModule : public phpcxx::Module {
public:
    int module_startup_called   = 0;
    int module_shutdown_called  = 0;
    int request_startup_called  = 0;
    int request_shutdown_called = 0;

    LoadOtherModule(const char* name, const char* version, phpcxx::Module& other)
        : phpcxx::Module(name, version), m_other(other)
    {
    }

protected:
    virtual bool moduleStartup() override
    {
        ++this->module_startup_called;
        return true;
    }

    virtual bool moduleShutdown() override
    {
        ++this->module_shutdown_called;
        return true;
    }

    virtual bool requestStartup() override
    {
        ++this->request_startup_called;
        return true;
    }

    virtual bool requestShutdown() override
    {
        ++this->request_shutdown_called;
        return true;
    }

    virtual std::vector<Module*> otherModules() override
    {
        return { &this->m_other };
    }

private:
    phpcxx::Module& m_other;
};

}

TEST(LifecycleTest, NormalRequest)
{
    MyModule ext("LifeCycle", "0.0");

    EXPECT_EQ(0, ext.module_startup_called);
    EXPECT_EQ(0, ext.module_shutdown_called);
    EXPECT_EQ(0, ext.request_startup_called);
    EXPECT_EQ(0, ext.request_shutdown_called);

    {
        TestSAPI sapi(std::cout, std::cerr);
        sapi.addModule(ext);

        sapi.initialize();
        EXPECT_EQ(1, ext.module_startup_called);
        EXPECT_EQ(0, ext.module_shutdown_called);
        EXPECT_EQ(0, ext.request_startup_called);
        EXPECT_EQ(0, ext.request_shutdown_called);

        sapi.run([&ext]() {
            EXPECT_EQ(1, ext.module_startup_called);
            EXPECT_EQ(0, ext.module_shutdown_called);
            EXPECT_EQ(1, ext.request_startup_called);
            EXPECT_EQ(0, ext.request_shutdown_called);
        });

        EXPECT_EQ(1, ext.module_startup_called);
        EXPECT_EQ(0, ext.module_shutdown_called);
        EXPECT_EQ(1, ext.request_startup_called);
        EXPECT_EQ(1, ext.request_shutdown_called);
    }

    EXPECT_EQ(1, ext.module_startup_called);
    EXPECT_EQ(1, ext.module_shutdown_called);
    EXPECT_EQ(1, ext.request_startup_called);
    EXPECT_EQ(1, ext.request_shutdown_called);
}

TEST(LifecycleTest, ErroredRequest)
{
    MyModule ext("LifeCycle", "0.0");

    EXPECT_EQ(0, ext.module_startup_called);
    EXPECT_EQ(0, ext.module_shutdown_called);
    EXPECT_EQ(0, ext.request_startup_called);
    EXPECT_EQ(0, ext.request_shutdown_called);

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addModule(ext);

        sapi.initialize();
        EXPECT_EQ(1, ext.module_startup_called);
        EXPECT_EQ(0, ext.module_shutdown_called);
        EXPECT_EQ(0, ext.request_startup_called);
        EXPECT_EQ(0, ext.request_shutdown_called);

        sapi.run([&ext]() {
            EXPECT_EQ(1, ext.module_startup_called);
            EXPECT_EQ(0, ext.module_shutdown_called);
            EXPECT_EQ(1, ext.request_startup_called);
            EXPECT_EQ(0, ext.request_shutdown_called);

            zend_error(E_ERROR, "Very fatal error");
        });

        EXPECT_EQ(1, ext.module_startup_called);
        EXPECT_EQ(0, ext.module_shutdown_called);
        EXPECT_EQ(1, ext.request_startup_called);
        EXPECT_EQ(1, ext.request_shutdown_called);

        EXPECT_EQ(out.str(), "");
        std::string errors = err.str();
        EXPECT_NE(errors.find("Very fatal error"), std::string::npos);
    }

    EXPECT_EQ(1, ext.module_startup_called);
    EXPECT_EQ(1, ext.module_shutdown_called);
    EXPECT_EQ(1, ext.request_startup_called);
    EXPECT_EQ(1, ext.request_shutdown_called);
}

TEST(LifecycleTest, MultipleRequests)
{
    MyModule ext("LifeCycle", "0.0");

    EXPECT_EQ(0, ext.module_startup_called);
    EXPECT_EQ(0, ext.module_shutdown_called);
    EXPECT_EQ(0, ext.request_startup_called);
    EXPECT_EQ(0, ext.request_shutdown_called);
    int n = 100;

    {
        TestSAPI sapi(std::cout, std::cerr);
        sapi.addModule(ext);

        sapi.initialize();
        EXPECT_EQ(1, ext.module_startup_called);
        EXPECT_EQ(0, ext.module_shutdown_called);
        EXPECT_EQ(0, ext.request_startup_called);
        EXPECT_EQ(0, ext.request_shutdown_called);

        for (int i=0; i<n; ++i) {
            sapi.run([&ext, &i]() {
                EXPECT_EQ(1,   ext.module_startup_called);
                EXPECT_EQ(0,   ext.module_shutdown_called);
                EXPECT_EQ(i+1, ext.request_startup_called);
                EXPECT_EQ(i,   ext.request_shutdown_called);
            });
        }

        EXPECT_EQ(1, ext.module_startup_called);
        EXPECT_EQ(0, ext.module_shutdown_called);
        EXPECT_EQ(n, ext.request_startup_called);
        EXPECT_EQ(n, ext.request_shutdown_called);
    }

    EXPECT_EQ(1, ext.module_startup_called);
    EXPECT_EQ(1, ext.module_shutdown_called);
    EXPECT_EQ(n, ext.request_startup_called);
    EXPECT_EQ(n, ext.request_shutdown_called);
}

TEST(LifecycleTest, AdditionalModule)
{
    {
        MyModule ext2("LifeCycle2", "0.0");
        LoadOtherModule ext1("LifeCycle1", "0.0", ext2);

        {
            TestSAPI sapi(std::cout, std::cerr);
            sapi.addModule(ext1);

            sapi.initialize();
            EXPECT_EQ(1, ext1.module_startup_called);
            EXPECT_EQ(1, ext2.module_startup_called);

            sapi.run([&ext1, &ext2]() {
                EXPECT_EQ(1, ext1.request_startup_called);
                EXPECT_EQ(1, ext2.request_startup_called);
            });
        }

        EXPECT_EQ(1, ext1.module_shutdown_called);
        EXPECT_EQ(1, ext2.module_shutdown_called);
    }

    {
        MyModule ext1("LifeCycle1", "0.0");
        MyModule ext2("LifeCycle2", "0.0");

        {
            TestSAPI sapi(std::cout, std::cerr);
            sapi.addModule(ext1);

            sapi.initialize();
            zend_startup_module(ext2.module());
            EXPECT_EQ(1, ext1.module_startup_called);
            EXPECT_EQ(1, ext2.module_startup_called);

            sapi.run([&ext1, &ext2]() {
                EXPECT_EQ(1, ext1.request_startup_called);
                // When a module is loaded after MINIT phase,
                // its request startup / shutdown functions
                // do not get registered
                // However, I would prefer not to rely upon
                // this behavior
                // EXPECT_EQ(0, ext2.request_startup_called);
            });

            sapi.run([&ext1, &ext2]() {
                EXPECT_EQ(2, ext1.request_startup_called);
                // See above
                // EXPECT_EQ(0, ext2.request_startup_called);
            });
        }

        EXPECT_EQ(1, ext1.module_shutdown_called);
        EXPECT_EQ(1, ext2.module_shutdown_called);
    }
}

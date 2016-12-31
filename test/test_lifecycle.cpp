#include <memory>
#include <iostream>
#include <sstream>
#include <gtest/gtest.h>
#include <Zend/zend.h>
#include "phpcxx/extension.h"
#include "testsapi.h"

class MyExtension : public phpcxx::Extension {
public:
    int module_startup_called   = 0;
    int module_shutdown_called  = 0;
    int request_startup_called  = 0;
    int request_shutdown_called = 0;

    using phpcxx::Extension::Extension;

protected:
    virtual void onModuleStartup() override
    {
        ++this->module_startup_called;
    }

    virtual void onModuleShutdown() override
    {
        ++this->module_shutdown_called;
    }

    virtual void onRequestStartup() override
    {
        ++this->request_startup_called;
    }

    virtual void onRequestShutdown() override
    {
        ++this->request_shutdown_called;
    }

};

TEST(LifecycleTest, NormalRequest)
{
    MyExtension ext("LifeCycle", "0.0");

    EXPECT_EQ(0, ext.module_startup_called);
    EXPECT_EQ(0, ext.module_shutdown_called);
    EXPECT_EQ(0, ext.request_startup_called);
    EXPECT_EQ(0, ext.request_shutdown_called);

    {
        TestSAPI sapi(std::cout, std::cerr);
        sapi.addExtension(ext);

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
    MyExtension ext("LifeCycle", "0.0");

    EXPECT_EQ(0, ext.module_startup_called);
    EXPECT_EQ(0, ext.module_shutdown_called);
    EXPECT_EQ(0, ext.request_startup_called);
    EXPECT_EQ(0, ext.request_shutdown_called);

    std::stringstream out;
    std::stringstream err;

    {
        TestSAPI sapi(out, err);
        sapi.addExtension(ext);

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
    MyExtension ext("LifeCycle", "0.0");

    EXPECT_EQ(0, ext.module_startup_called);
    EXPECT_EQ(0, ext.module_shutdown_called);
    EXPECT_EQ(0, ext.request_startup_called);
    EXPECT_EQ(0, ext.request_shutdown_called);
    int n = 100;

    {
        TestSAPI sapi(std::cout, std::cerr);
        sapi.addExtension(ext);

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

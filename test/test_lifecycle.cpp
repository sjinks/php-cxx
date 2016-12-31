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

    EXPECT_EQ(ext.module_startup_called,   0);
    EXPECT_EQ(ext.module_shutdown_called,  0);
    EXPECT_EQ(ext.request_startup_called,  0);
    EXPECT_EQ(ext.request_shutdown_called, 0);

    {
        TestSAPI sapi(std::cout, std::cerr);
        sapi.addExtension(ext);

        sapi.initialize();
        EXPECT_EQ(ext.module_startup_called,   1);
        EXPECT_EQ(ext.module_shutdown_called,  0);
        EXPECT_EQ(ext.request_startup_called,  0);
        EXPECT_EQ(ext.request_shutdown_called, 0);

        sapi.run([&ext]() {
            EXPECT_EQ(ext.module_startup_called,   1);
            EXPECT_EQ(ext.module_shutdown_called,  0);
            EXPECT_EQ(ext.request_startup_called,  1);
            EXPECT_EQ(ext.request_shutdown_called, 0);
        });

        EXPECT_EQ(ext.module_startup_called,   1);
        EXPECT_EQ(ext.module_shutdown_called,  0);
        EXPECT_EQ(ext.request_startup_called,  1);
        EXPECT_EQ(ext.request_shutdown_called, 1);
    }

    EXPECT_EQ(ext.module_startup_called,   1);
    EXPECT_EQ(ext.module_shutdown_called,  1);
    EXPECT_EQ(ext.request_startup_called,  1);
    EXPECT_EQ(ext.request_shutdown_called, 1);
}

TEST(LifecycleTest, ErroredRequest)
{
    MyExtension ext("LifeCycle", "0.0");

    EXPECT_EQ(ext.module_startup_called,   0);
    EXPECT_EQ(ext.module_shutdown_called,  0);
    EXPECT_EQ(ext.request_startup_called,  0);
    EXPECT_EQ(ext.request_shutdown_called, 0);

    {
        std::stringstream out;
        std::stringstream err;
        TestSAPI sapi(out, err);
        sapi.addExtension(ext);

        sapi.initialize();
        EXPECT_EQ(ext.module_startup_called,   1);
        EXPECT_EQ(ext.module_shutdown_called,  0);
        EXPECT_EQ(ext.request_startup_called,  0);
        EXPECT_EQ(ext.request_shutdown_called, 0);

        sapi.run([&ext]() {
            EXPECT_EQ(ext.module_startup_called,   1);
            EXPECT_EQ(ext.module_shutdown_called,  0);
            EXPECT_EQ(ext.request_startup_called,  1);
            EXPECT_EQ(ext.request_shutdown_called, 0);

            zend_error(E_ERROR, "Very fatal error");
        });

        EXPECT_EQ(ext.module_startup_called,   1);
        EXPECT_EQ(ext.module_shutdown_called,  0);
        EXPECT_EQ(ext.request_startup_called,  1);
        EXPECT_EQ(ext.request_shutdown_called, 1);

        EXPECT_EQ(out.str(), "");
        std::string errors = err.str();
        EXPECT_NE(errors.find("Very fatal error"), std::string::npos);
    }

    EXPECT_EQ(ext.module_startup_called,   1);
    EXPECT_EQ(ext.module_shutdown_called,  1);
    EXPECT_EQ(ext.request_startup_called,  1);
    EXPECT_EQ(ext.request_shutdown_called, 1);
}

TEST(LifecycleTest, MultipleRequests)
{
    MyExtension ext("LifeCycle", "0.0");

    EXPECT_EQ(ext.module_startup_called,   0);
    EXPECT_EQ(ext.module_shutdown_called,  0);
    EXPECT_EQ(ext.request_startup_called,  0);
    EXPECT_EQ(ext.request_shutdown_called, 0);
    int n = 100;

    {
        TestSAPI sapi(std::cout, std::cerr);
        sapi.addExtension(ext);

        sapi.initialize();
        EXPECT_EQ(ext.module_startup_called,   1);
        EXPECT_EQ(ext.module_shutdown_called,  0);
        EXPECT_EQ(ext.request_startup_called,  0);
        EXPECT_EQ(ext.request_shutdown_called, 0);

        for (int i=0; i<n; ++i) {
            sapi.run([&ext, &i]() {
                EXPECT_EQ(ext.module_startup_called,   1);
                EXPECT_EQ(ext.module_shutdown_called,  0);
                EXPECT_EQ(ext.request_startup_called,  i+1);
                EXPECT_EQ(ext.request_shutdown_called, i);
            });
        }

        EXPECT_EQ(ext.module_startup_called,   1);
        EXPECT_EQ(ext.module_shutdown_called,  0);
        EXPECT_EQ(ext.request_startup_called,  n);
        EXPECT_EQ(ext.request_shutdown_called, n);
    }

    EXPECT_EQ(ext.module_startup_called,   1);
    EXPECT_EQ(ext.module_shutdown_called,  1);
    EXPECT_EQ(ext.request_startup_called,  n);
    EXPECT_EQ(ext.request_shutdown_called, n);
}

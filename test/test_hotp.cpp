#include <cstdint>
#include <cstring>
#include <ctime>
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <phpcxx/phpclass.h>
#include "phpcxx/fcall.h"
#include "phpcxx/phpclass.h"
#include "phpcxx/callable.h"
#include "phpcxx/module.h"
#include "testsapi.h"
#include "globals.h"

namespace {

class HOTP : public phpcxx::ClassImplementationBase {
public:

    static phpcxx::Value generateByCounter(phpcxx::Parameters& p)
    {
        phpcxx::Value key = p[1];
        phpcxx::Value ctr = p[0];
        char buf[8];

        std::uint64_t c   = ctr.asLong();
#ifndef WORDS_BIGENDIAN
        c = (std::uint64_t(htonl(std::uint32_t(c))) << 32) | htonl(c >> 32);
#endif
        std::memcpy(buf, &c, sizeof(buf));
        phpcxx::string s(buf, 8);

        phpcxx::Value hmac = phpcxx::call("hash_hmac", "sha1", s, key, true);
        return static_cast<zend_long>(HOTP::getCode(hmac));
    }

    static phpcxx::Value generateByTime(phpcxx::Parameters& p)
    {
        phpcxx::Value key    = p[0];
        phpcxx::Value window = p[1];
        phpcxx::Value tstamp;

        if (p.size() >= 3) {
            tstamp = p[2];
        }
        else {
            tstamp = static_cast<zend_long>(std::time(nullptr));
        }

        phpcxx::Value counter = (tstamp / window).asLong();
        return phpcxx::FCall("self::generateByCounter")(counter, key);
    }

    static phpcxx::Value generateByTimeWindow(phpcxx::Parameters& p)
    {
        phpcxx::Value key    = p[0];
        phpcxx::Value window = p[1];
        phpcxx::Value min    = p.size() > 2 ? p[2] : -1;
        phpcxx::Value max    = p.size() > 3 ? p[3] :  1;
        phpcxx::Value tstamp = p.size() > 4 ? p[4] : static_cast<zend_long>(std::time(nullptr));

        phpcxx::Array result;
        phpcxx::Value counter = (tstamp / window).asLong() + min.asLong();

        phpcxx::FCall call("self::generateByCounter");

        for (int i=min.asLong(); i<=max.asLong(); ++i, ++counter) {
            result[nullptr] = call(counter, key);
        }

        return phpcxx::Value(result.pzval());
    }

private:
    static std::uint32_t getCode(const phpcxx::Value& hmac)
    {
        phpcxx::string s = hmac.asString();
        unsigned int offset = s[19] & 0x0F;
        unsigned char c1    = s[offset+0];
        unsigned char c2    = s[offset+1];
        unsigned char c3    = s[offset+2];
        unsigned char c4    = s[offset+3];
        return ((c1 & 0x7F) << 24) | (c2 << 16) | (c3 << 8) | c4;
    }
};

class MyModule : public phpcxx::Module {
public:
    using phpcxx::Module::Module;

protected:
    std::unique_ptr<phpcxx::PhpClass<HOTP> > m_hotp;

    virtual std::vector<std::shared_ptr<phpcxx::ClassBase> > classes() override
    {
        phpcxx::PhpClass<HOTP> hotp("HOTP", 0);
        hotp.addStaticMethod<&HOTP::generateByCounter>("generateByCounter");
        hotp.addStaticMethod<&HOTP::generateByTime>("generateByTime");
        hotp.addStaticMethod<&HOTP::generateByTimeWindow>("generateByTimeWindow");
        return {
            std::make_shared<phpcxx::ClassBase>(hotp)
        };

    }
};

}

TEST(HOTP, TestHOTP)
{
    std::stringstream out;
    std::stringstream err;

    {
        MyModule ext("HOTP", nullptr);
        TestSAPI sapi(out, err);
        sapi.addModule(ext);
        sapi.initialize();

        sapi.run([]() {
            runPhpCode("echo HOTP::generateByCounter(1, '12345678901234567890'), \"\n\";");
            runPhpCode("echo HOTP::generateByTime('12345678901234567890', 30, 2000000000), \"\n\";");
            runPhpCode("$r = HOTP::generateByTimeWindow('12345678901234567890', 30, -5, 5, 5); foreach ($r as $x) echo $x, \"\n\";");
        });

        EXPECT_EQ("1094287082\n2069279037\n425293533\n942152854\n1576851516\n1289488204\n1663094451\n1284755224\n1094287082\n137359152\n1726969429\n1640338314\n868254676\n", out.str()); out.str(std::string());
        EXPECT_EQ("", err.str()); err.str(std::string());
    }

}

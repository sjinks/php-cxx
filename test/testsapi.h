#ifndef MY_SAPI_H
#define MY_SAPI_H

#include <ostream>
#include <functional>

namespace phpcxx {
class Extension;
}

class TestSAPI {
public:
    TestSAPI(std::ostream& out, std::ostream& err);
    ~TestSAPI();

    void initialize();
    void addExtension(phpcxx::Extension& ext);
    void run(std::function<void(void)> callback);

    [[gnu::nonnull]] void setOutputStream(std::ostream* os);
    [[gnu::nonnull]] void setErrorStream(std::ostream* os);
private:
    bool m_initialized;
};

#endif /* SAPI_H_ */

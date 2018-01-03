#ifndef MY_SAPI_H
#define MY_SAPI_H

#include <ostream>
#include <functional>

namespace phpcxx {
class Module;
}

class TestSAPI {
public:
    TestSAPI(std::ostream& out, std::ostream& err);
    ~TestSAPI();

    void addModule(phpcxx::Module& ext);
    void run(std::function<void(void)> callback);

    [[gnu::nonnull]] void setOutputStream(std::ostream* os);
    [[gnu::nonnull]] void setErrorStream(std::ostream* os);
private:
    bool m_initialized;
    bool m_in_request;
};

#endif /* SAPI_H_ */

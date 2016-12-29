#ifndef MY_SAPI_H
#define MY_SAPI_H

#include <ostream>
#include <functional>

class TestSAPI {
public:
    TestSAPI(std::ostream& out, std::ostream& err);
    ~TestSAPI();

    void run(std::function<void(void)> callback);

    [[gnu::nonnull]] void setOutputStream(std::ostream* os);
    [[gnu::nonnull]] void setErrorStream(std::ostream* os);
};

#endif /* SAPI_H_ */

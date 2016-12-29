#include <iostream>
#include <string>

#include "emallocallocator.h"
#include "extension.h"
#include "function.h"
#include "parameters.h"
#include "phpexception.h"
#include "string.h"
#include "value.h"
#include "vector.h"

#include "sapi.h"

static phpcxx::Extension ext("wow", "1.0");
static MySAPI sapi;

static phpcxx::Value myFunction(phpcxx::Parameters& p)
{
    std::cout << "HELLO!" << std::endl;
    return p[0] + " - " + p[1] + "\n";
}

void doWork()
{
    try {
        phpcxx::Value v = phpcxx::Value("myFunction")(4, 5);
        std::cout << v.toString() << std::endl;
    }
    catch (const phpcxx::PhpException& e) {
        std::cerr << e.getClass() << std::endl;
        std::cerr << e.message() << std::endl;
        std::cerr << e.file() << std::endl;
        std::cerr << e.line() << std::endl;
        phpcxx::Value("print_r")(e.trace());
    }
}

int main(int argc, char** argv)
{
    ext.addFunction(
        phpcxx::createFunction<myFunction>("myFunction",
            {
                phpcxx::byValue("a", phpcxx::ArgumentType::Callable),
                phpcxx::byValue("b", phpcxx::ArgumentType::Callable, false),
                phpcxx::byValue("c", phpcxx::ArgumentType::Callable, false)
            }
        ).setTypeHint(phpcxx::ArgumentType::Any)
    );

    ext.registerExtension(ext);

    sapi.run(doWork);
    return 0;
}

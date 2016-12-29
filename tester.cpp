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

static phpcxx::Value myFunction(phpcxx::Parameters&)
{
    return 0;
}

static void runPhpCode(const phpcxx::string& code)
{
    char* desc = zend_make_compiled_string_description("eval'd code");
    zend_eval_stringl_ex(const_cast<char*>(code.c_str()), code.size(), nullptr, desc, 1);
    efree(desc);
}

void doWork()
{
    runPhpCode("throw new Exception('111');");
    std::cout << "Done" << std::endl;
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

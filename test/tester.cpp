#include <iostream>
#include <string>

#include "phpcxx/emallocallocator.h"
#include "phpcxx/extension.h"
#include "phpcxx/function.h"
#include "phpcxx/parameters.h"
#include "phpcxx/phpexception.h"
#include "phpcxx/string.h"
#include "phpcxx/value.h"
#include "phpcxx/vector.h"

#include "testsapi.h"

static phpcxx::Extension ext("wow", "1.0");
static TestSAPI sapi(std::cout, std::cerr);

//static phpcxx::Value myFunction(phpcxx::Parameters&)
//{
//    return 0;
//}
//
//static void runPhpCode(const phpcxx::string& code)
//{
//    char* desc = zend_make_compiled_string_description("eval'd code");
//    zend_eval_stringl_ex(const_cast<char*>(code.c_str()), code.size(), nullptr, desc, 1);
//    efree(desc);
//}

void doWork()
{
}

int main(int argc, char** argv)
{
//    ext.addFunction(
//        phpcxx::createFunction<myFunction>("myFunction",
//            {
//                phpcxx::byValue("a", phpcxx::ArgumentType::Callable),
//                phpcxx::byValue("b", phpcxx::ArgumentType::Callable, false),
//                phpcxx::byValue("c", phpcxx::ArgumentType::Callable, false).setNullable(false),
//                phpcxx::Argument("d").setType(phpcxx::ArgumentType::Array).setNullable(true).setByRef(false).setVariadic(false)
//            }
//        ).setTypeHint(phpcxx::ArgumentType::Any)
//    );

    ext.registerExtension(ext);

    sapi.run(doWork);

    return 0;
}

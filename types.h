#ifndef PHPCXX_TYPES_H
#define PHPCXX_TYPES_H

#include <Zend/zend_types.h>

namespace phpcxx {

enum class Type : zend_uchar {
    Undefined       = IS_UNDEF,
    Null            = IS_NULL,
    False           = IS_FALSE,
    True            = IS_TRUE,
    Integer         = IS_LONG,
    Double          = IS_DOUBLE,
    String          = IS_STRING,
    Array           = IS_ARRAY,
    Object          = IS_OBJECT,
    Resource        = IS_RESOURCE,
    Reference       = IS_REFERENCE,
    Constant        = IS_CONSTANT,
    ConstantAST     = IS_CONSTANT_AST,
    Bool            = _IS_BOOL,
    Callable        = IS_CALLABLE,
    Indirect        = IS_INDIRECT,
    Pointer         = IS_PTR
};

enum class ArgumentType : zend_uchar {
    Any       = IS_UNDEF,
    Bool      = _IS_BOOL,
    Integer   = IS_LONG,
    Double    = IS_DOUBLE,
    String    = IS_STRING,
    Array     = IS_ARRAY,
    Object    = IS_OBJECT,
    Resource  = IS_RESOURCE,
    Callable  = IS_CALLABLE
};

enum class CopyPolicy {
    Assign,
    Reference,
    Copy,
    Wrap,
    Duplicate
};

enum class AssignPolicy {
    AssignVariable,
    AssignTemporary
};

class Parameters;
class Value;

using InternalFunction    = void(*)(struct _zend_execute_data* execute_data, struct _zval_struct* return_value);
using FunctionPrototypeNN = void(*)();
using FunctionPrototypeNV = void(*)(phpcxx::Parameters&);
using FunctionPrototypeVN = Value(*)();
using FunctionPrototypeVV = Value(*)(phpcxx::Parameters&);

}



#endif /* PHPCXX_TYPES_H */

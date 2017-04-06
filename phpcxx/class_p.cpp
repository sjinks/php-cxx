#include <cassert>
#include <cstring>

extern "C" {
#include <Zend/zend_API.h>
#include <Zend/zend_compile.h>
#include <Zend/zend_inheritance.h>
#include <Zend/zend_objects_API.h>
}

#include "classbase.h"
#include "class_p.h"

static zend_object_handlers ce_handlers;

void phpcxx::ClassPrivate::initializeClass()
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, this->m_name, this->methods());

    this->m_ce = zend_register_internal_class_ex(&ce, this->m_parent_ce);
    if (UNEXPECTED(!this->m_ce)) {
        zend_error(E_ERROR, "Failed to register class %s", this->m_name);
        return;
    }

    for (auto&& c : this->m_interfaces) {
        zend_do_implement_interface(this->m_ce, c);
    }

    zend_declare_property_null(this->m_ce, ZEND_STRL("\xFF"), ZEND_ACC_PRIVATE);

    this->m_ce->create_object = ClassPrivate::create_object;

    std::memcpy(&ce_handlers, zend_get_std_object_handlers(), sizeof(ce_handlers));
    ce_handlers.free_obj = ClassPrivate::free_object;

    this->m_this              = zend_string_alloc(2*sizeof(this)-1, 1);
    ZSTR_VAL(this->m_this)[0] = '\0';
    ZSTR_LEN(this->m_this)    = 0;

    auto self = this;
    std::memcpy(ZSTR_VAL(this->m_this) + sizeof(this), &self, sizeof(this));
    this->m_ce->info.user.doc_comment = this->m_this;

    this->registerConstants();
}

zend_object* phpcxx::ClassPrivate::create_object(zend_class_entry* ce)
{
    zend_object* res = nullptr;
    if (ce->parent && ce->parent->create_object) {
        res = ce->parent->create_object(ce);
    }
    else {
        res = static_cast<zend_object*>(ecalloc(1, sizeof(zend_object) + zend_object_properties_size(ce)));
        zend_object_std_init(res, ce);
        object_properties_init(res, ce);
    }

    res->handlers = &ce_handlers;

    zend_class_entry* c = ce;
    while (c->parent && (!c->info.user.doc_comment || ZSTR_LEN(c->info.user.doc_comment))) {
        c = c->parent;
    }

    if (UNEXPECTED(!c->info.user.doc_comment || ZSTR_LEN(c->info.user.doc_comment))) {
        zend_error(E_ERROR, "create_object() failed for %s", ce->name ? ZSTR_VAL(ce->name) : "<unknown class>");
        return nullptr;
    }

    const char* comment = ZSTR_VAL(c->info.user.doc_comment);
    ClassPrivate* self  = *reinterpret_cast<ClassPrivate**>(const_cast<char*>(comment + sizeof(ClassPrivate*)));
    auto native_class   = self->q_ptr->construct();

    zval tmp;
    ZVAL_PTR(&tmp, native_class);

    zval this_ptr;
    ZVAL_OBJ(&this_ptr, res);
    zend_update_property(ce, &this_ptr, ZEND_STRL("\xFF"), &tmp);
    return res;
}

void phpcxx::ClassPrivate::free_object(zend_object* object)
{
    zval z;
    ZVAL_OBJ(&z, object);
    zval* z1 = zend_read_property(object->ce, &z, ZEND_STRL("\xFF"), 1, NULL);

    if (IS_PTR == Z_TYPE_P(z1)) {
        auto* native_class = reinterpret_cast<ClassBase*>(Z_PTR_P(z1));
        phpcxx::emdestroy(native_class);

        SEPARATE_ZVAL(z1);
        zval_dtor(z1);
        ZVAL_NULL(z1);
    }

    zend_object_std_dtor(object);
}

zend_function_entry* phpcxx::ClassPrivate::methods()
{
    static zend_function_entry empty;
    assert(empty.fname == nullptr && empty.handler == nullptr && empty.arg_info == nullptr);

    this->m_funcs = this->q_ptr->methods();
    this->m_zf.reset(new zend_function_entry[this->m_funcs.size()+1]);
    zend_function_entry* ptr = this->m_zf.get();

    for (auto&& f : this->m_funcs) {
        *ptr++ = f.getFE();
    }

    *ptr = empty;
    return this->m_zf.get();
}

void phpcxx::ClassPrivate::registerConstants()
{
    std::vector<ClassConstant> constants = this->q_ptr->constants();

    for (auto&& c : constants) {
        zval& z = c.value();
        Z_TRY_ADDREF(z);
        if (SUCCESS != zend_declare_class_constant(this->m_ce, c.name(), std::strlen(c.name()), &z)) {
            Z_TRY_DELREF(z);
        }
    }
}

#ifndef PHPCXX_EXTENSION_H
#define PHPCXX_EXTENSION_H

#include "phpcxx.h"

#include <memory>
#include "function.h"

struct _zend_module_entry;

namespace phpcxx {

class ExtensionPrivate;

class PHPCXX_EXPORT Extension {
public:
	Extension(const char* name, const char* version);

	Extension(const Extension&) = delete;
	Extension(Extension&&) = delete;
	Extension& operator=(const Extension&) = delete;
	Extension& operator=(Extension&&) = delete;

	virtual ~Extension() noexcept;

	struct _zend_module_entry* module();

	static void registerExtension(Extension& other);

	void addFunction(const Function& f);

protected:
	virtual void onModuleStartup()   {}
	virtual void onModuleShutdown()  {}
	virtual void onRequestStartup()  {}
	virtual void onRequestShutdown() {}
	virtual void onModuleInfo();

private:
	friend class ExtensionPrivate;
	std::unique_ptr<ExtensionPrivate> d_ptr;
};

}

#endif /* PHPCXX_EXTENSION_H */

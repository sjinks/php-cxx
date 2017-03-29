#ifndef PHPCXX_MODULEMAP_P_H
#define PHPCXX_MODULEMAP_P_H

#include "phpcxx.h"

#include <atomic>
#include <cstring>
#include <string>
#include <mutex>
#include <unordered_map>
#include "module_p.h"

namespace phpcxx {

class PHPCXX_HIDDEN ModuleMap {
public:
    static ModuleMap& instance()
    {
        static ModuleMap self;
        return self;
    }

    ModulePrivate* byId(int module)
    {
        std::unique_lock<std::mutex> locker(this->m_mutex);
        auto it = this->m_i2e.find(module);
        return it != this->m_i2e.end() ? it->second : nullptr;
    }

    [[gnu::nonnull]] void add(ModulePrivate* m)
    {
        std::unique_lock<std::mutex> locker(this->m_mutex);
        this->m_n2e.emplace(m->entry.name, m);
    }

    void remove(int module)
    {
        std::unique_lock<std::mutex> locker(this->m_mutex);
        auto it = this->m_i2e.find(module);
        if (it != this->m_i2e.end()) {
            ModulePrivate* e = it->second;
            this->m_i2e.erase(it);
            this->m_n2e.erase(e->entry.name);
        }
    }

    [[gnu::nonnull]] ModulePrivate* mapIdToModule(const char* name, int module)
    {
        std::unique_lock<std::mutex> locker(this->m_mutex);
        auto it = this->m_n2e.find(std::string(name));
        if (it != this->m_n2e.end()) {
            this->m_i2e.emplace(module, it->second);
            return it->second;
        }

        return nullptr;
    }

    void clear()
    {
        std::unique_lock<std::mutex> locker(this->m_mutex);
        this->m_n2e.clear();
        this->m_i2e.clear();
        this->m_ctr = 0;
    }

    void onGINIT()
    {
        ++this->m_ctr;
    }

    unsigned int onGSHUTDOWN()
    {
        unsigned int v = --this->m_ctr;
        if (!v) {
            this->clear();
        }

        return v;
    }

private:
    std::mutex m_mutex;
    std::unordered_map<std::string, ModulePrivate*> m_n2e;
    std::unordered_map<int, ModulePrivate*> m_i2e;
    std::atomic<unsigned int> m_ctr;

    ModuleMap() : m_ctr(0) {}

    ModuleMap(const ModuleMap&) = delete;
    ModuleMap& operator=(const ModuleMap&) = delete;
};

}

#endif /* PHPCXX_MODULEMAP_P_H */

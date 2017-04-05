#ifndef PHPCXX_EMALLOCALLOCATOR_H
#define PHPCXX_EMALLOCALLOCATOR_H

#include "phpcxx.h"

#include <cassert>
#include <cstddef>
#include <memory>
#include <new>

namespace phpcxx {

template<typename T>
class PHPCXX_EXPORT EMallocAllocator {
public:
    typedef T value_type;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;

    template<class U>
    struct rebind { typedef EMallocAllocator<U> other; };

    EMallocAllocator(void) noexcept = default;

    template<typename U>
    EMallocAllocator(const EMallocAllocator<U>&) noexcept
    {}

    T* allocate(std::size_t cnt)
    {
#ifdef PHPCXX_DEBUG
        assert(SG(sapi_started));
#endif
        return static_cast<T*>(safe_emalloc(cnt, sizeof(T), 0)); // Zend throws a fatal error on OOM
    }

    void deallocate(T* p, std::size_t)
    {
        efree(p);
    }

    template<typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        ::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template<class U>
    void destroy(U* p)
    {
        p->~U();
    }
};

template<typename T, typename U>
static inline bool operator==(const EMallocAllocator<T>&, const EMallocAllocator<U>&) noexcept
{
    return true;
}

template<typename T, typename U>
static inline bool operator!=(const EMallocAllocator<T>& lhs, const EMallocAllocator<U>& rhs) noexcept
{
    return !operator==(lhs, rhs);
}

template<typename T, typename... Args>
static inline T* emcreate(Args&&... args)
{
    using Allocator = phpcxx::EMallocAllocator<T>;
    using Traits    = std::allocator_traits<Allocator>;
    Allocator alloc;
    typename Traits::pointer p = Traits::allocate(alloc, 1);
    Traits::construct(alloc, p, std::forward<Args>(args)...);
    return p;
}

template<typename T>
static inline void emdestroy(T* p)
{
    using Allocator = phpcxx::EMallocAllocator<T>;
    using Traits    = std::allocator_traits<Allocator>;
    Allocator alloc;
    Traits::destroy(alloc, p);
    Traits::deallocate(alloc, p, 1);
}

struct PHPCXX_EXPORT emdeleter {
    template<typename T>
    void operator()(T* ptr) const { emdestroy(ptr); }
};

}

#endif /* PHPCXX_EMALLOCALLOCATOR_H */

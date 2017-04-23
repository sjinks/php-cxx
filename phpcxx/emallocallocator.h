#ifndef PHPCXX_EMALLOCALLOCATOR_H
#define PHPCXX_EMALLOCALLOCATOR_H

#include "phpcxx.h"

extern "C" {
#include <Zend/zend.h>
#include <main/SAPI.h>
}

#include <cassert>
#include <cstddef>
#include <memory>
#include <new>

#ifdef PHPCXX_DEBUG
#include "valgrind/memcheck.h"
#endif

namespace phpcxx {

/**
 * @brief `emalloc()`-based allocator
 */
template<typename T>
class PHPCXX_EXPORT EMallocAllocator {
public:
    typedef T value_type; ///< Value type

    /**@deprecated Definition for old compilers */
    typedef size_t size_type;           ///< Size type
    /**@deprecated Definition for old compilers */
    typedef ptrdiff_t difference_type;  ///< Difference type
    /**@deprecated Definition for old compilers */
    typedef T* pointer;                 ///< Pointer type
    /**@deprecated Definition for old compilers */
    typedef const T* const_pointer;     ///< Constant pointer type
    /**@deprecated Definition for old compilers */
    typedef T& reference;               ///< Reference type
    /**@deprecated Definition for old compilers */
    typedef const T& const_reference;   ///< Constant reference type

    /**
     * @brief Provides a way to obtain an allocator for a different type `U`
     * @deprecated
     */
    template<class U>
    struct rebind { typedef EMallocAllocator<U> other; };

    /**
     * @brief Default constructor
     */
    EMallocAllocator(void) noexcept = default;

    /**
     * @brief Copy constructor
     */
    template<typename U>
    EMallocAllocator(const EMallocAllocator<U>&) noexcept
    {}

    /**
     * @brief Allocates `cnt * sizeof(T)` bytes of uninitialized storage
     * @param cnt Number of objects to allocate
     * @return Pointer to the allocated storage
     * @warning It is an error to call this function if the SAPI is not started
     *
     * Uses `safe_emalloc()` for memory allocation. Never returns `nullptr`
     * because a fatal error is thrown upon memory allocation failure
     */
    [[gnu::malloc]] T* allocate(std::size_t cnt)
    {
#ifdef PHPCXX_DEBUG
        assert(SG(sapi_started));
        T* res = static_cast<T*>(safe_emalloc(cnt, sizeof(T), 0));
        assert(res != nullptr);
        VALGRIND_MAKE_MEM_UNDEFINED(res, cnt * sizeof(T));
        return res;
#endif
        return static_cast<T*>(safe_emalloc(cnt, sizeof(T), 0)); // Zend throws a fatal error on OOM
    }

    /**
     * @brief Deallocates the storage referenced by the pointer `p`
     * @param p Storage to deallocate
     * @param cnt Number of the allocated objects
     * @warning `p` must be a pointer obtained by an earlier call to `allocate()`
     */
    void deallocate(T* p, std::size_t cnt)
    {
#ifdef PHPCXX_DEBUG
        VALGRIND_MAKE_MEM_NOACCESS(p, cnt * sizeof(T));
#endif
        static_cast<void>(cnt);
        efree(p);
    }

    /**
     * @deprecated Included for compatibility with old compilers
     * @brief Constructs an object of type `U` in allocated uninitialized storage pointed to by `p`, using placement-new
     * @param p Allocated storage
     * @param args Constructor arguments
     */
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        ::new(reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    /**
     * @deprecated Included for compatibility with old compilers
     * @brief Calls the destructor of the object pointed to by p
     * @param p Object to be destroyed
     */
    template<class U>
    void destroy(U* p)
    {
        p->~U();
    }
};

/**
 * @brief Compares two allocators for equality
 * @return Always `true` (allocators are stateless, thus two `EMallocAllocator`'s are equal)
 */
template<typename T, typename U>
static inline bool operator==(const EMallocAllocator<T>&, const EMallocAllocator<U>&) noexcept
{
    return true;
}

/**
 * @brief Compares two allocators for inequality
 * @return Always `false` (allocators are stateless, thus two `EMallocAllocator`'s are equal)
 */
template<typename T, typename U>
static inline bool operator!=(const EMallocAllocator<T>& lhs, const EMallocAllocator<U>& rhs) noexcept
{
    return !operator==(lhs, rhs);
}

/**
 * @brief Allocates and initializes an object `T` with arguments @a args using `EMallocAllocator`
 * @param args Constructor arguments
 * @return Alloacted an initialized object
 */
template<typename T, typename... Args>
[[gnu::malloc]] static inline T* emcreate(Args&&... args)
{
    using Allocator = phpcxx::EMallocAllocator<T>;
    using Traits    = std::allocator_traits<Allocator>;
    Allocator alloc;
    typename Traits::pointer p = Traits::allocate(alloc, 1);
    assert(p != nullptr);
    Traits::construct(alloc, p, std::forward<Args>(args)...);
    return p;
}

/**
 * @brief Destroys and deallocates the object pointed to by @a p previously created with `emcreate()`
 */
template<typename T>
static inline void emdestroy(T* p)
{
    using Allocator = phpcxx::EMallocAllocator<T>;
    using Traits    = std::allocator_traits<Allocator>;
    Allocator alloc;
    Traits::destroy(alloc, p);
    Traits::deallocate(alloc, p, 1);
}

/**
 * @brief Functor wrapping `emdestroy()`
 */
struct PHPCXX_EXPORT emdeleter {
    template<typename T>
    void operator()(T* ptr) const { emdestroy(ptr); }
};

}

#endif /* PHPCXX_EMALLOCALLOCATOR_H */

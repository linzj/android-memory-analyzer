#ifndef SPECIALALLOCATOR_H
#define SPECIALALLOCATOR_H
#pragma once

template <class _Tp>
class SpecialAllocator {
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _Tp* pointer;
    typedef const _Tp* const_pointer;
    typedef _Tp& reference;
    typedef const _Tp& const_reference;
    typedef _Tp value_type;

    template <typename _Tp1>
    struct rebind {
        typedef SpecialAllocator<_Tp1> other;
    };

    SpecialAllocator() throw() {}

    SpecialAllocator(const SpecialAllocator&) throw() {}

    template <typename _Tp1>
    SpecialAllocator(const SpecialAllocator<_Tp1>&) throw() {}

    ~SpecialAllocator() throw() {}

    pointer
    address(reference __x) const { return &__x; }

    const_pointer
    address(const_reference __x) const { return &__x; }

    // NB: __n is permitted to be 0.  The C++ standard says nothing
    // about what the return value is when __n == 0.
    pointer
    allocate(size_type __n, const void* = 0);

    // __p is not permitted to be a null pointer.
    void
    deallocate(pointer __p, size_type);

    size_type
    max_size() const throw()
    {
        return size_t(-1) / sizeof(_Tp);
    }

    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    // 402. wrong new expression in [some_] allocator::construct
    void
    construct(pointer __p, const _Tp& __val)
    {
        ::new ((void*)__p) _Tp(__val);
    }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    template <typename... _Args>
    void
    construct(pointer __p, _Args&&... __args);
#endif

    void
    destroy(pointer __p)
    {
        __p->~_Tp();
    }
};

#endif /* SPECIALALLOCATOR_H */

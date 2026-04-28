#ifndef MSL_MEMORY_H_
#define MSL_MEMORY_H_

namespace std
{

template <class T>
class allocator
{
public:
    allocator() { }
    void destroy(T*);
    void deallocate(T* p, unsigned long) { ::operator delete(p); }
};

template <class ForwardIt, class Size, class T>
inline ForwardIt uninitialized_fill_n(ForwardIt first, Size count, const T& value)
{
    for (; count--; ++first)
    {
        if (first != NULL)
        {
            *first = value;
        }
    }
    return first;
}

template <class InputIterator, class ForwardIterator>
inline ForwardIterator __uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
{
    ForwardIterator __save = result;

    for (; first != last; ++first, ++result)
    {
        *result = *first;
    }
    return result;
}

template <class T, bool A, bool B>
struct __uninitialized_copy_helper
{
    static T* uninitialized_copy(T* first, T* last, T* result)
    {
        return __uninitialized_copy(first, last, result);
    }
};

template <class T>
struct __uninitialized_copy_helper<T, true, false>
{
    static T* uninitialized_copy(T* first, T* last, T* result)
    {
        for (; first < last; ++result, ++first)
            *result = *first;
        return result;
    }
};

template <class T>
inline T* uninitialized_copy(T* first, T* last, T* result)
{
    return __uninitialized_copy_helper<T, true, false>::uninitialized_copy(first, last, result);
}

} // namespace std

namespace Metrowerks
{
namespace details
{

template <class First, class Second, int tag>
class compressed_pair_imp
{
    First first_;
    Second second_;

public:
    compressed_pair_imp() { }
    compressed_pair_imp(const First& f)
        : first_(f)
        , second_()
    {
    }
    compressed_pair_imp(const First& f, const Second& s)
        : first_(f)
        , second_(s)
    {
    }
    First& first() { return first_; }
    Second& second() { return second_; }
};

template <class First, class Second>
class compressed_pair_imp<First, Second, 1> : private First
{
    Second second_;

public:
    compressed_pair_imp()
        : First()
        , second_()
    {
    }
    compressed_pair_imp(const Second& s)
        : First()
        , second_(s)
    {
    }
    First& first() { return *this; }
    Second& second() { return second_; }
};

} // namespace details
} // namespace Metrowerks

#endif

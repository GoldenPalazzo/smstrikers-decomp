#ifndef MSL_TREE_H_
#define MSL_TREE_H_

#include "PowerPC_EABI_Support/MSL_C++/MSL_Common/utility.h"
#include "PowerPC_EABI_Support/MSL_C++/MSL_Common/msl_memory.h"

namespace std
{

template <class Arg1, class Arg2, class Result>
struct binary_function
{
};

template <class T>
struct less : public binary_function<T, T, bool>
{
};

template <int N>
class __red_black_tree
{
public:
    struct anchor
    {
        void* left_;
    };
};

template <class T, class Compare, class Allocator>
class __tree : private __red_black_tree<1>
{
public:
    struct node
    {
    };
    __tree(const Compare& comp, const Allocator& alloc);
    void alloc();
    std::allocator<node>& node_alloc();
    void clear();

private:
    Metrowerks::details::compressed_pair_imp<Allocator, unsigned long, 1> alloc_;
    Metrowerks::details::compressed_pair_imp<std::allocator<node>, __red_black_tree<1>::anchor, 1> node_alloc_;
    Metrowerks::details::compressed_pair_imp<Compare, node*, 1> comp_;
};

template <class Key, class Value, class Compare = less<Key>, class Allocator = allocator<pair<const Key, Value> > >
class map
{
public:
    class value_compare : public binary_function<pair<const Key, Value>, pair<const Key, Value>, bool>
    {
    protected:
        Compare comp;
    };

    map() : tree_(value_compare(), Allocator()) {}

private:
    __tree<pair<const Key, Value>, value_compare, Allocator> tree_;
};

} // namespace std

#endif

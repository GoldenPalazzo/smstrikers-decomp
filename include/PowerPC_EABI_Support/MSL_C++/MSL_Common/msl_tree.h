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
    struct node_base;
    struct anchor
    {
        void* left_;
        anchor()
            : left_(0)
        {
        }
    };
    struct node_base : public anchor
    {
        void* right_;
        void* parent_;
    };

    static void rotate_left(node_base* x, node_base*& root);
    static void rotate_right(node_base* x, node_base*& root);
};

template <int N>
void __red_black_tree<N>::rotate_left(node_base* x, node_base*& root)
{
    node_base* y = (node_base*)x->right_;
    if (root == x)
    {
        root = y;
    }
    x->right_ = y->left_;
    node_base* yl = (node_base*)y->left_;
    if (yl != 0)
    {
        yl->parent_ = (void*)((unsigned long)x | ((unsigned long)yl->parent_ & 1));
    }
    y->parent_ = (void*)(((unsigned long)x->parent_ & ~1) | ((unsigned long)y->parent_ & 1));
    node_base* parent = (node_base*)((unsigned long)x->parent_ & ~1);
    if (x == (node_base*)parent->left_)
    {
        parent->left_ = y;
    }
    else
    {
        parent->right_ = y;
    }
    y->left_ = x;
    x->parent_ = (void*)((unsigned long)y | ((unsigned long)x->parent_ & 1));
}

template <int N>
void __red_black_tree<N>::rotate_right(node_base* x, node_base*& root)
{
    node_base* y = (node_base*)x->left_;
    if (root == x)
    {
        root = y;
    }
    x->left_ = y->right_;
    node_base* yr = (node_base*)y->right_;
    if (yr != 0)
    {
        yr->parent_ = (void*)((unsigned long)x | ((unsigned long)yr->parent_ & 1));
    }
    y->parent_ = (void*)(((unsigned long)x->parent_ & ~1) | ((unsigned long)y->parent_ & 1));
    node_base* parent = (node_base*)((unsigned long)x->parent_ & ~1);
    if (x == (node_base*)parent->left_)
    {
        parent->left_ = y;
    }
    else
    {
        parent->right_ = y;
    }
    y->right_ = x;
    x->parent_ = (void*)((unsigned long)y | ((unsigned long)x->parent_ & 1));
}

template <class T, class Compare, class Allocator>
class __tree : private __red_black_tree<1>
{
public:
    struct node : public __red_black_tree<1>::node_base
    {
        T data_;
    };

    class iterator
    {
    public:
        node* ptr_;
        iterator(node* p)
            : ptr_(p)
        {
        }
    };

    __tree(const Compare& comp, const Allocator& alloc);
    void alloc();
    std::allocator<node>& node_alloc();
    void clear();
    void destroy(node* n);

    template <class Key>
    iterator find(const Key& x);

private:
    Metrowerks::details::compressed_pair_imp<Allocator, unsigned long, 1> alloc_;
    Metrowerks::details::compressed_pair_imp<std::allocator<node>, __red_black_tree<1>::anchor, 1> node_alloc_;
    Metrowerks::details::compressed_pair_imp<Compare, node*, 0> comp_;
};

template <class T, class Compare, class Allocator>
__tree<T, Compare, Allocator>::__tree(const Compare& comp, const Allocator& alloc)
    : alloc_()
    , node_alloc_()
    , comp_(comp, (node*)&node_alloc_.second())
{
}

template <class T, class Compare, class Allocator>
template <class Key>
typename __tree<T, Compare, Allocator>::iterator
__tree<T, Compare, Allocator>::find(const Key& x)
{
    node* i = (node*)node_alloc_.second().left_;
    node* j = (node*)&node_alloc_.second();
    while (i != 0)
    {
        if (!(i->data_.first < x))
        {
            j = i;
            i = (node*)i->left_;
        }
        else
        {
            i = (node*)i->right_;
        }
    }
    if (j == (node*)&node_alloc_.second() || x < j->data_.first)
    {
        return iterator((node*)&node_alloc_.second());
    }
    return iterator(j);
}

template <class Key, class Value, class Compare = less<Key>, class Allocator = allocator<pair<const Key, Value> > >
class map
{
public:
    class value_compare : public binary_function<pair<const Key, Value>, pair<const Key, Value>, bool>
    {
    protected:
        Compare comp;
    };

    typedef typename __tree<pair<const Key, Value>, value_compare, Allocator>::iterator iterator;

    map()
        : tree_(value_compare(), Allocator())
    {
    }

    iterator find(const Key& x)
    {
        return tree_.find(x);
    }

    __tree<pair<const Key, Value>, value_compare, Allocator> tree_;
};

} // namespace std

#endif

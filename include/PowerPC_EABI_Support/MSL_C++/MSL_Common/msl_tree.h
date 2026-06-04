#ifndef MSL_TREE_H_
#define MSL_TREE_H_

#include "PowerPC_EABI_Support/MSL_C++/MSL_Common/utility.h"
#include "PowerPC_EABI_Support/MSL_C++/MSL_Common/msl_memory.h"
#include "stdio.h"
#include "stdlib.h"

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
    static void balance_insert(node_base* x, node_base* root);
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

// Color is stored in bit 0 of parent_ (1 = red, 0 = black); the pointer is the
// remaining bits. These mirror the rotate_left/rotate_right encoding above.
#define RB_PARENT(n)    ((node_base*)((unsigned long)(n)->parent_ & ~1))
#define RB_IS_RED(n)    (((unsigned long)(n)->parent_ & 1) == 1)
#define RB_SET_RED(n)   ((n)->parent_ = (void*)((unsigned long)(n)->parent_ | 1))
#define RB_SET_BLACK(n) ((n)->parent_ = (void*)((unsigned long)(n)->parent_ & ~1))

template <int N>
void __red_black_tree<N>::balance_insert(node_base* x, node_base* root)
{
    RB_SET_RED(x);
    while (x != root && RB_IS_RED(RB_PARENT(x)))
    {
        node_base* p = RB_PARENT(x);
        node_base* g = RB_PARENT(p);
        if (p == (node_base*)g->left_)
        {
            node_base* y = (node_base*)g->right_;
            if (y != 0 && RB_IS_RED(y))
            {
                RB_SET_BLACK(p);
                RB_SET_BLACK(y);
                RB_SET_RED(g);
                x = g;
            }
            else
            {
                if (x == (node_base*)p->right_)
                {
                    x = p;
                    rotate_left(x, root);
                }
                RB_SET_BLACK(RB_PARENT(x));
                RB_SET_RED(RB_PARENT(RB_PARENT(x)));
                rotate_right(RB_PARENT(RB_PARENT(x)), root);
            }
        }
        else
        {
            node_base* y = (node_base*)g->left_;
            if (y != 0 && RB_IS_RED(y))
            {
                RB_SET_BLACK(p);
                RB_SET_BLACK(y);
                RB_SET_RED(g);
                x = g;
            }
            else
            {
                if (x == (node_base*)p->left_)
                {
                    x = p;
                    rotate_right(x, root);
                }
                RB_SET_BLACK(RB_PARENT(x));
                RB_SET_RED(RB_PARENT(RB_PARENT(x)));
                rotate_left(RB_PARENT(RB_PARENT(x)), root);
            }
        }
    }
    RB_SET_BLACK(root);
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
    Allocator& alloc();
    std::allocator<node>& node_alloc();
    void clear();
    void destroy(node* n);

    template <class Key>
    iterator find(const Key& x);

    template <class Key, class Value>
    T& find_or_insert(const Key& key);

    node* insert_node_at(node* p, bool leftchild, bool is_leftmost, const T& x);

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

template <class T, class Compare, class Allocator>
template <class Key, class Value>
T& __tree<T, Compare, Allocator>::find_or_insert(const Key& key)
{
    node* prev = 0;
    node* p = (node*)&node_alloc_.second();
    node* n = (node*)node_alloc_.second().left_;
    bool leftchild = true;
    bool is_leftmost = true;

    while (n != 0)
    {
        p = n;
        if (key < n->data_.first)
        {
            n = (node*)n->left_;
            leftchild = true;
        }
        else
        {
            prev = n;
            n = (node*)n->right_;
            leftchild = false;
            is_leftmost = false;
        }
    }

    if (prev == 0 || prev->data_.first < key)
    {
        T x(key, Value());
        return insert_node_at(p, leftchild, is_leftmost, x)->data_;
    }

    return prev->data_;
}

template <class T, class Compare, class Allocator>
typename __tree<T, Compare, Allocator>::node*
__tree<T, Compare, Allocator>::insert_node_at(node* p, bool leftchild, bool is_leftmost, const T& x)
{
    if (alloc_.second() > 0xfffffffeU)
    {
        fprintf(stderr, "tree::insert length error\n");
        abort();
    }

    node* new_node = (node*)::operator new(sizeof(node));
    if (new_node == 0)
    {
        fprintf(stderr, "Memory allocation failure");
        abort();
    }

    new (&new_node->data_) T(x);
    new_node->right_ = 0;
    new_node->left_ = 0;
    new_node->parent_ = (void*)((unsigned long)p | ((unsigned long)new_node->parent_ & 1));

    if (leftchild)
        p->left_ = new_node;
    else
        p->right_ = new_node;

    ++alloc_.second();
    balance_insert(new_node, (node_base*)node_alloc_.second().left_);

    if (is_leftmost)
        comp_.second() = new_node;

    return new_node;
}

#undef RB_PARENT
#undef RB_IS_RED
#undef RB_SET_RED
#undef RB_SET_BLACK

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

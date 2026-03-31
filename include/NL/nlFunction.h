#ifndef _FEFUNCTION_H_
#define _FEFUNCTION_H_

#include "NL/nlMemory.h"
#include "types.h"

enum Tag
{
    EMPTY = 0,
    FREE_FUNCTION = 1,
    FUNCTOR = 2,
};

struct FunctorBase
{
    virtual ~FunctorBase() { };
    virtual void Invoke() = 0;
    virtual FunctorBase* Clone() const = 0;
};

// Placeholder for Bind argument forwarding
template <int N>
struct Placeholder
{
};

extern Placeholder<0> placeholder0;

template <typename ReturnType, typename ParamType>
class Function1
{
public:
    struct FunctorBase
    {
        virtual ~FunctorBase() { };
        virtual ReturnType operator()(ParamType) = 0;
        virtual FunctorBase* Clone() const = 0;
    };

    template <typename BindType>
    struct FunctorImpl : public FunctorBase
    {
        BindType mBind;
        FunctorImpl() { }
        FunctorImpl(const BindType& b)
            : mBind(b)
        {
        }
        virtual ~FunctorImpl() { }
        virtual ReturnType operator()(ParamType arg);
        virtual FunctorBase* Clone() const { return new (nlMalloc(sizeof(FunctorImpl), 8, false)) FunctorImpl(*this); }
    };

    enum Tag mTag; // offset 0x0, size 0x4
    union
    {
        ReturnType (*mFreeFunction)(ParamType); // offset 0x4, size 0x4
        FunctorBase* mFunctor;                  // offset 0x4, size 0x4
    };

    inline void operator()(ParamType arg)
    {
        if ((bool)mTag)
        {
            if (mTag == FREE_FUNCTION)
            {
                mFreeFunction(arg);
            }
            else
            {
                (*mFunctor)(arg);
            }
        }
    }
}; // total size: 0x8

template <typename ReturnType>
class Function0
{
public:
    struct FunctorBase
    {
        virtual ~FunctorBase() { };
        virtual void Invoke() = 0;
        virtual FunctorBase* Clone() const = 0;
    };

    template <typename BindType>
    struct FunctorImpl : public FunctorBase
    {
        BindType mBind;
        FunctorImpl() { }
        FunctorImpl(const BindType& b)
            : mBind(b)
        {
        }
        virtual ~FunctorImpl() { }
        virtual void Invoke() { }
        virtual FunctorBase* Clone() const { return new (nlMalloc(sizeof(FunctorImpl), 8, false)) FunctorImpl(*this); }
    };

    enum Tag mTag; // offset 0x0, size 0x4
    union
    {
        ReturnType (*mFreeFunction)(); // offset 0x4, size 0x4
        FunctorBase* mFunctor;         // offset 0x4, size 0x4
    };

    Function0()
    {
    }

    Function0(const Function0& other)
        : mTag(other.mTag)
    {
        if (mTag == FREE_FUNCTION)
        {
            mFreeFunction = other.mFreeFunction;
        }
        else if (mTag == FUNCTOR)
        {
            mFunctor = other.mFunctor->Clone();
        }
    }

    ~Function0()
    {
        if (mTag == FUNCTOR)
        {
            delete mFunctor;
        }
        mTag = EMPTY;
    }

}; // total size: 0x8

template <typename T>
class Function : public Function1<void, T>
{
public:
    Function() { }

    template <typename BindType>
    Function(const BindType& bind)
    {
        typedef typename Function1<void, T>::template FunctorImpl<BindType> ImplType;
        ImplType* impl = new (nlMalloc(sizeof(ImplType), 8, false)) ImplType(bind);
        mTag = FUNCTOR;
        mFunctor = impl;
    }

    ~Function()
    {
        if (mTag == FUNCTOR)
        {
            delete mFunctor;
        }
        mTag = EMPTY;
    }

    Function& operator=(const Function& other)
    {
        if (mTag == FUNCTOR)
        {
            delete mFunctor;
        }
        mTag = EMPTY;
        mTag = other.mTag;
        if (mTag == FREE_FUNCTION)
        {
            mFreeFunction = other.mFreeFunction;
        }
        else if (mTag == FUNCTOR)
        {
            mFunctor = other.mFunctor->Clone();
        }
        return *this;
    }
}; // total size: 0x8

template <typename R, typename P>
class Function<R(P)> : public Function1<R, P>
{
public:
    Function() { }

    template <typename BindType>
    Function(const BindType& bind)
    {
        typedef typename Function1<R, P>::template FunctorImpl<BindType> ImplType;
        ImplType* impl = new (nlMalloc(sizeof(ImplType), 8, false)) ImplType(bind);
        mTag = FUNCTOR;
        mFunctor = impl;
    }

    ~Function()
    {
        if (mTag == FUNCTOR)
        {
            delete mFunctor;
        }
        mTag = EMPTY;
    }

    Function& operator=(const Function& other)
    {
        if (mTag == FUNCTOR)
        {
            delete mFunctor;
        }
        mTag = EMPTY;
        mTag = other.mTag;
        if (mTag == FREE_FUNCTION)
        {
            mFreeFunction = other.mFreeFunction;
        }
        else if (mTag == FUNCTOR)
        {
            mFunctor = other.mFunctor->Clone();
        }
        return *this;
    }
};

typedef void FnVoidVoid();

template <>
class Function<FnVoidVoid> : public Function0<void>
{
public:
    Function()
        : Function0<void>()
    {
    }

    template <typename T>
    Function(T);

    Function& operator=(const Function& other)
    {
        if (mTag == FUNCTOR)
        {
            delete mFunctor;
        }
        mTag = EMPTY;
        mTag = other.mTag;
        if (mTag == FREE_FUNCTION)
        {
            mFreeFunction = other.mFreeFunction;
        }
        else if (mTag == FUNCTOR)
        {
            mFunctor = other.mFunctor->Clone();
        }
        return *this;
    }
}; // total size: 0x8

// Bind template
template <typename R, typename F, typename A>
struct BindExp1
{
    F mFuncPtr;
    A mArg;
};

template <typename R, typename F, typename A>
BindExp1<R, F, A> Bind(F fn, const A& arg);

#endif // _FEFUNCTION_H_

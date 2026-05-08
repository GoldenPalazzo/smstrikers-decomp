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
    virtual void operator()() = 0;
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
        virtual ReturnType operator()() = 0;
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
        virtual ReturnType operator()() { FORCE_DONT_INLINE; }
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

    template <typename BindType>
    Function0(const BindType& bind)
    {
        typedef FunctorImpl<BindType> ImplType;
        mTag = FUNCTOR;
        mFunctor = new (nlMalloc(sizeof(ImplType), 8, false)) ImplType(bind);
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
    Function()
        : Function1<void, T>()
    {
    }

    template <typename BindType>
    Function(const BindType& bind)
    {
        typedef typename Function1<void, T>::template FunctorImpl<BindType> ImplType;
        mTag = FUNCTOR;
        mFunctor = new (nlMalloc(sizeof(ImplType), 8, false)) ImplType(bind);
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
    Function()
        : Function1<R, P>()
    {
        mTag = EMPTY;
    }

    template <typename BindType>
    Function(const BindType& bind)
    {
        typedef typename Function1<R, P>::template FunctorImpl<BindType> ImplType;
        mTag = FUNCTOR;
        mFunctor = new (nlMalloc(sizeof(ImplType), 8, false)) ImplType(bind);
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

    Function(Tag t)
    {
        mTag = t;
    }

    template <typename T>
    Function(T bind)
        : Function0<void>(bind)
    {
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

// MemFun template
namespace Detail
{
template <typename R, typename MemPtr>
struct MemFunImpl
{
    MemPtr mMemFun;
    MemFunImpl() { }
    MemFunImpl(MemPtr fn)
        : mMemFun(fn)
    {
    }
};
} // namespace Detail

template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*fn)());

template <typename T, typename R, typename P>
Detail::MemFunImpl<R, void (T::*)(P)> MemFun(void (T::*fn)(P));

// Bind template
template <typename R, typename F, typename A>
struct BindExp1
{
    F mFuncPtr;
    A mArg;

    BindExp1() { }
    BindExp1(F fn, A a)
        : mFuncPtr(fn)
        , mArg(a)
    {
    }
};

template <typename R, typename F, typename A>
BindExp1<R, F, A> Bind(F fn, const A& arg)
{
    BindExp1<R, F, A> result;
    result.mFuncPtr = fn;
    result.mArg = arg;
    return result;
}

template <typename R, typename F, typename A, typename B>
struct BindExp2
{
    F mFunction;
    A mT0;
    B mT1;

    BindExp2() { }
    BindExp2(F fn, const A& t0, const B& t1)
        : mFunction(fn)
        , mT0(t0)
        , mT1(t1)
    {
    }
};

template <typename R, typename F, typename A, typename B>
BindExp2<R, F, A, B> Bind(F fn, const A& t0, const B& t1)
{
    return BindExp2<R, F, A, B>(fn, t0, t1);
}

#endif // _FEFUNCTION_H_

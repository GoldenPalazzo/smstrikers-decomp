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
#ifdef FUNCTION1_SPLIT_BODIES
        virtual ReturnType operator()(ParamType arg);
        virtual FunctorBase* Clone() const;
#include "NL/nlFunction1Dtor.h"
#else
        virtual ~FunctorImpl() { }
        virtual ReturnType operator()(ParamType arg);
        virtual FunctorBase* Clone() const { return new (nlMalloc(sizeof(FunctorImpl), 8, false)) FunctorImpl(*this); }
#endif
    };

    enum Tag mTag; // offset 0x0, size 0x4
    union
    {
        ReturnType (*mFreeFunction)(ParamType); // offset 0x4, size 0x4
        FunctorBase* mFunctor;                  // offset 0x4, size 0x4
    };

    inline void operator()(ParamType arg)
    {
        int tag = mTag;
        if (((u32)((-tag) | tag) >> 31) > 0)
        {
            if (tag == FREE_FUNCTION)
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
#ifdef FUNCTION0_SPLIT_BODIES
        virtual ReturnType operator()();
        virtual FunctorBase* Clone() const;
#include "NL/nlFunction0Dtor.h"
#else
        virtual ~FunctorImpl() { }
        virtual ReturnType operator()() { FORCE_DONT_INLINE; }
        virtual FunctorBase* Clone() const { return new (nlMalloc(sizeof(FunctorImpl), 8, false)) FunctorImpl(*this); }
#endif
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

template <typename R, typename P1, typename P2, typename P3>
class Function<R(P1, P2, P3)>
{
public:
    struct FunctorBase
    {
        virtual ~FunctorBase() { };
        virtual R operator()(P1, P2, P3) = 0;
        virtual FunctorBase* Clone() const = 0;
    };

    enum Tag mTag; // offset 0x0, size 0x4
    union
    {
        R (*mFreeFunction)(P1, P2, P3); // offset 0x4, size 0x4
        FunctorBase* mFunctor;          // offset 0x4, size 0x4
    };

    Function()
    {
        mTag = EMPTY;
    }

    Function(R (*fn)(P1, P2, P3))
    {
        mTag = FREE_FUNCTION;
        mFreeFunction = fn;
    }

    ~Function()
    {
        if (mTag == FUNCTOR)
        {
            delete mFunctor;
        }
        mTag = EMPTY;
    }
}; // total size: 0x8

typedef void FnVoidVoid();

template <>
class Function<FnVoidVoid> : public Function0<void>
{
public:
    Function()
        : Function0<void>()
    {
#ifdef FESLIDEMENU_FUNCTION_CTOR_INIT
        mTag = EMPTY;
#endif
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
#ifdef FEPOPUPMENU_FUNCTION_ASSIGN_DONT_INLINE
        FORCE_DONT_INLINE;
#endif
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

    template <typename T>
    R operator()(T* obj) const
    {
        return (obj->*mMemFun)();
    }

    template <typename T, typename P>
    R operator()(T* obj, P arg) const
    {
        return (obj->*mMemFun)(arg);
    }
};
} // namespace Detail

#ifndef MEMFUN_NO_DECL
template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*fn)());

template <typename T, typename R, typename P>
Detail::MemFunImpl<R, void (T::*)(P)> MemFun(void (T::*fn)(P));
#endif

// Bind template
template <typename R, typename F, typename A>
struct BindExp1
{
    F mFuncPtr;
    A mArg;

    BindExp1() { }
    BindExp1(F fn, const A& a)
        : mFuncPtr(fn)
        , mArg(a)
    {
    }

    R operator()()
    {
        return mFuncPtr(mArg);
    }

    template <typename P>
    R operator()(P)
    {
        return mFuncPtr(mArg);
    }
};

#ifndef BIND_NO_DECL
template <typename R, typename F, typename A>
BindExp1<R, F, A> Bind(F fn, const A& arg)
{
    BindExp1<R, F, A> result;
    result.mFuncPtr = fn;
    result.mArg = arg;
    return result;
}

#endif

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

    R operator()()
    {
        return mFunction(mT0, mT1);
    }

    template <typename P>
    R operator()(P arg)
    {
        return DoCall(arg, mT0, mT1);
    }

private:
    template <typename P, typename B1>
    R DoCall(P arg, const Placeholder<0>&, const B1& t1)
    {
        return mFunction(arg, t1);
    }

    template <typename P, typename A1>
    R DoCall(P arg, const A1& t0, const Placeholder<0>&)
    {
        return mFunction(t0, arg);
    }

    template <typename P, typename A1, typename B1>
    R DoCall(P, const A1& t0, const B1& t1)
    {
        return mFunction(t0, t1);
    }
};

#ifndef BIND_NO_DECL
template <typename R, typename F, typename A, typename B>
BindExp2<R, F, A, B> Bind(F fn, const A& t0, const B& t1)
{
    return BindExp2<R, F, A, B>(fn, t0, t1);
}
#endif

#ifdef FUNCTION1_SPLIT_BODIES
template <typename ReturnType, typename ParamType>
template <typename BindType>
inline ReturnType Function1<ReturnType, ParamType>::FunctorImpl<BindType>::operator()(ParamType arg)
{
    return mBind(arg);
}

template <typename ReturnType, typename ParamType>
template <typename BindType>
inline typename Function1<ReturnType, ParamType>::FunctorBase*
Function1<ReturnType, ParamType>::FunctorImpl<BindType>::Clone() const
{
    return new (nlMalloc(sizeof(FunctorImpl), 8, false)) FunctorImpl(mBind);
}
#endif

#ifdef FUNCTION0_SPLIT_BODIES
template <typename ReturnType>
template <typename BindType>
inline ReturnType Function0<ReturnType>::FunctorImpl<BindType>::operator()()
{
    return mBind();
}

template <typename ReturnType>
template <typename BindType>
inline typename Function0<ReturnType>::FunctorBase* Function0<ReturnType>::FunctorImpl<BindType>::Clone() const
{
    return new (nlMalloc(sizeof(FunctorImpl), 8, false)) FunctorImpl(mBind);
}
#endif

#endif // _FEFUNCTION_H_

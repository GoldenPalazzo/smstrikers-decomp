#ifndef _NLFUNCTION_H_
#define _NLFUNCTION_H_

#include "NL/nlMemory.h"
#include "types.h"

enum Tag
{
    EMPTY = 0,
    FREE_FUNCTION = 1,
    FUNCTOR = 2,
};

template <typename T>
struct IsVoid
{
    enum
    {
        value = false,
    };
};

template <>
struct IsVoid<void>
{
    enum
    {
        value = true,
    };
};

template <bool Value>
struct BoolToType
{
};

namespace Detail
{
template <typename R, typename MemPtr>
struct MemFunImpl
{
private:
    MemPtr mMemFun;

public:
    MemFunImpl(MemPtr function)
        : mMemFun(function)
    {
    }

    template <typename T>
    R operator()(T* object) const
    {
        return (object->*mMemFun)();
    }

    template <typename T, typename P>
    R operator()(T* object, P argument) const
    {
        return (object->*mMemFun)(argument);
    }
};
} // namespace Detail

/* Original nlFunction.h source order: parameterized overload, then arity 0. */
template <typename T, typename R, typename P>
Detail::MemFunImpl<R, R (T::*)(P)> MemFun(R (T::*function)(P))
{
    return Detail::MemFunImpl<R, R (T::*)(P)>(function);
}

template <typename T, typename R>
Detail::MemFunImpl<R, R (T::*)()> MemFun(R (T::*function)())
{
    return Detail::MemFunImpl<R, R (T::*)()>(function);
}

template <typename Signature>
class Function;

typedef void FnVoidVoid();

template <typename ReturnType, typename P1>
class Function1;

/* Function<P1> is the one-argument shorthand for Function<void(P1)>. */
#define NLF_GENERATE_PRIMARY_WRAPPER
#include "NL/detail/nlFunctionPreProcTemplate.h"
#undef NLF_GENERATE_PRIMARY_WRAPPER

/* Function0<ReturnType> and Function<ReturnType()>. */
#define NLF_CLASS Function0
#define NLF_TEMPLATE_PARAMETERS typename ReturnType
#define NLF_TEMPLATE_ARGUMENTS ReturnType
#define NLF_PARAMETER_TYPES
#define NLF_PARAMETER_DECLARATIONS
#define NLF_ARGUMENT_NAMES
#define NLF_COMMA_PARAMETER_DECLARATIONS
#define NLF_COMMA_ARGUMENT_NAMES
#define NLF_ARITY 0
#include "NL/detail/nlFunctionPreProcTemplate.h"
#undef NLF_ARITY
#undef NLF_COMMA_ARGUMENT_NAMES
#undef NLF_COMMA_PARAMETER_DECLARATIONS
#undef NLF_ARGUMENT_NAMES
#undef NLF_PARAMETER_DECLARATIONS
#undef NLF_PARAMETER_TYPES
#undef NLF_TEMPLATE_ARGUMENTS
#undef NLF_TEMPLATE_PARAMETERS
#undef NLF_CLASS

/* Function1<ReturnType, P1> and Function<ReturnType(P1)>. */
#define NLF_CLASS Function1
#define NLF_TEMPLATE_PARAMETERS typename ReturnType, typename P1
#define NLF_TEMPLATE_ARGUMENTS ReturnType, P1
#define NLF_PARAMETER_TYPES P1
#define NLF_PARAMETER_DECLARATIONS P1 p0
#define NLF_ARGUMENT_NAMES p0
#define NLF_COMMA_PARAMETER_DECLARATIONS , P1 p0
#define NLF_COMMA_ARGUMENT_NAMES , p0
#define NLF_ARITY 1
#include "NL/detail/nlFunctionPreProcTemplate.h"
#undef NLF_ARITY
#undef NLF_COMMA_ARGUMENT_NAMES
#undef NLF_COMMA_PARAMETER_DECLARATIONS
#undef NLF_ARGUMENT_NAMES
#undef NLF_PARAMETER_DECLARATIONS
#undef NLF_PARAMETER_TYPES
#undef NLF_TEMPLATE_ARGUMENTS
#undef NLF_TEMPLATE_PARAMETERS
#undef NLF_CLASS

/* Function3<ReturnType, P1, P2, P3> and the signature wrapper. */
#define NLF_CLASS Function3
#define NLF_TEMPLATE_PARAMETERS typename ReturnType, typename P1, typename P2, typename P3
#define NLF_TEMPLATE_ARGUMENTS ReturnType, P1, P2, P3
#define NLF_PARAMETER_TYPES P1, P2, P3
#define NLF_PARAMETER_DECLARATIONS P1 p1, P2 p2, P3 p3
#define NLF_ARGUMENT_NAMES p1, p2, p3
#define NLF_COMMA_PARAMETER_DECLARATIONS , P1 p1, P2 p2, P3 p3
#define NLF_COMMA_ARGUMENT_NAMES , p1, p2, p3
#define NLF_ARITY 3
#include "NL/detail/nlFunctionPreProcTemplate.h"
#undef NLF_ARITY
#undef NLF_COMMA_ARGUMENT_NAMES
#undef NLF_COMMA_PARAMETER_DECLARATIONS
#undef NLF_ARGUMENT_NAMES
#undef NLF_PARAMETER_DECLARATIONS
#undef NLF_PARAMETER_TYPES
#undef NLF_TEMPLATE_ARGUMENTS
#undef NLF_TEMPLATE_PARAMETERS
#undef NLF_CLASS

#endif // _NLFUNCTION_H_

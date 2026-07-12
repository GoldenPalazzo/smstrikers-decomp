#ifndef _NLFUNCTIONREAP_H_
#define _NLFUNCTIONREAP_H_

// A discardable template reference used to place a functor destructor in an
// earlier MWCC instantiation wave without retaining any helper code at link.
template <typename T>
void nlFunctionReap(T* function)
{
    FORCE_DONT_INLINE;
    function->~T();
}

template <typename T, typename P>
void nlFunction1Emit(T* function, P arg)
{
    FORCE_DONT_INLINE;
    (*function)(arg);
    function->Clone();
}

template <typename T>
void nlFunction0Emit(T* function)
{
    FORCE_DONT_INLINE;
    (*function)();
    function->Clone();
}

#endif // _NLFUNCTIONREAP_H_

// MemFun template bodies. Include after nlFunction.h.
// Define MEMFUN_USE_CONST before inclusion if MWCC adds const to member fn ptrs in this TU.
#ifdef MEMFUN_USE_CONST
template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*fn)() const)
{
    return Detail::MemFunImpl<R, void (T::*)()>((void (T::*)())(fn));
}

template <typename T, typename R, typename P>
Detail::MemFunImpl<R, void (T::*)(P)> MemFun(void (T::*fn)(P) const)
{
    return Detail::MemFunImpl<R, void (T::*)(P)>((void (T::*)(P))(fn));
}
#else
template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*fn)())
{
    return Detail::MemFunImpl<R, void (T::*)()>(fn);
}

template <typename T, typename R, typename P>
Detail::MemFunImpl<R, void (T::*)(P)> MemFun(void (T::*fn)(P))
{
    return Detail::MemFunImpl<R, void (T::*)(P)>(fn);
}
#endif

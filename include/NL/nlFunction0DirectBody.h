#ifndef _NLFUNCTION0DIRECTBODY_H_
#define _NLFUNCTION0DIRECTBODY_H_

#ifndef FUNCTION0_DIRECT_BODY_BIND
#error FUNCTION0_DIRECT_BODY_BIND must name the Function0 bind type
#endif

template <>
inline void Function0<void>::FunctorImpl<FUNCTION0_DIRECT_BODY_BIND>::operator()()
{
    FORCE_DONT_INLINE;
    (mBind.mArg->*mBind.mFuncPtr.mMemFun)();
}

template <>
inline Function0<void>::FunctorBase* Function0<void>::FunctorImpl<FUNCTION0_DIRECT_BODY_BIND>::Clone() const
{
    return new (nlMalloc(sizeof(FunctorImpl), 8, false)) FunctorImpl(mBind);
}

#endif // _NLFUNCTION0DIRECTBODY_H_

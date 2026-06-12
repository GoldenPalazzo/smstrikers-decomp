// Function1::FunctorImpl Clone/operator() bodies (dual-mode fragment).
//
// In-class mode (F1BODY_INCLASS, included by nlFunction.h inside the class
// when FUNCTION1_SPLIT_BODIES is defined): Clone body in-class, operator()
// declared only.
//
// Out-of-class mode: the TU defines F1BODY_RET / F1BODY_PARAM / F1BODY_BIND
// and includes this file again (after its Bind typedefs) to give operator()
// a weak per-TU body.
//
// Both bodies live in this one file so they share a single linkonce section;
// Clone's in-class body parses first, so Clone is emitted first (matches
// target object layout). The dtor body lives in nlFunction1Dtor.h so it
// lands in its own section after this one.
#ifdef F1BODY_INCLASS
        virtual ReturnType operator()(ParamType arg);
        virtual FunctorBase* Clone() const;
#else
template <>
inline F1BODY_RET Function1<F1BODY_RET, F1BODY_PARAM>::FunctorImpl<F1BODY_BIND>::operator()(F1BODY_PARAM)
{
    (mBind.mT0->*mBind.mFunction.mMemFun)(mBind.mT1);
}

template <>
inline Function1<F1BODY_RET, F1BODY_PARAM>::FunctorBase* Function1<F1BODY_RET, F1BODY_PARAM>::FunctorImpl<F1BODY_BIND>::Clone() const
{
    return new (nlMalloc(sizeof(FunctorImpl), 8, false)) FunctorImpl(mBind);
}
#endif

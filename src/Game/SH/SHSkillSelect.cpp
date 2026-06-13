#include "Game/SH/SHSkillSelect.h"
#include "NL/nlFunction.h"

typedef BindExp1<void, void (*)(bool), bool> BindExp1_vfb;
typedef Function0<void>::FunctorImpl<BindExp1_vfb> FunctorImpl_vfb;

template <>
void Function0<void>::FunctorImpl<BindExp1_vfb>::operator()()
{
    mBind.mFuncPtr(mBind.mArg);
}

// Explicit Clone spec (rather than the header-inlined one) so Clone emits into
// this TU's .text alongside operator(), matching the target's [Clone, __cl]
// section grouping. Construct from mBind (not *this) to use the BindType ctor
// and avoid emitting an out-of-line copy constructor.
template <>
Function0<void>::FunctorBase* Function0<void>::FunctorImpl<BindExp1_vfb>::Clone() const
{
    return new ((FunctorImpl_vfb*)nlMalloc(sizeof(FunctorImpl_vfb), 8, false)) FunctorImpl_vfb(mBind);
}

// Reconstruction stub (never called): anchors the weak Bind/FunctorImpl
// instantiations so they emit in this TU. Bind (referenced first) emits last,
// matching the target .text order [Clone, __cl, __dt, Bind].
static void _instantiate(void (*fn)(bool), const bool& arg)
{
    BindExp1_vfb bind = Bind<void, void (*)(bool), bool>(fn, arg);
    FunctorImpl_vfb* f = new ((FunctorImpl_vfb*)nlMalloc(sizeof(FunctorImpl_vfb), 8, false)) FunctorImpl_vfb(bind);
    (void)f;
}

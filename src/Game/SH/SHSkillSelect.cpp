#include "Game/SH/SHSkillSelect.h"
#include "NL/nlFunction.h"

typedef BindExp1<void, void (*)(bool), bool> BindExp1_vfb;
typedef Function0<void>::FunctorImpl<BindExp1_vfb> FunctorImpl_vfb;

template <>
void Function0<void>::FunctorImpl<BindExp1_vfb>::operator()()
{
    mBind.mFuncPtr(mBind.mArg);
}

static void _instantiate(const BindExp1_vfb& bind)
{
    FunctorImpl_vfb* f = new ((FunctorImpl_vfb*)nlMalloc(sizeof(FunctorImpl_vfb), 8, false)) FunctorImpl_vfb(bind);
    (void)f;
}

template BindExp1<void, void (*)(bool), bool> Bind<void, void (*)(bool), bool>(void (*)(bool), const bool&);

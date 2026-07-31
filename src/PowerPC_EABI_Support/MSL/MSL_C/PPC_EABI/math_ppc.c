#include "PowerPC_EABI_Support/MSL_C/PPC_EABI/math_ppc.h"
#include "fdlibm.h"

__declspec(weak) float acosf(float x)
{
    return acos(x);
}

__declspec(weak) float sinf(float x)
{
    return sin(x);
}

__declspec(weak) float tanf(float x)
{
    return tan(x);
}

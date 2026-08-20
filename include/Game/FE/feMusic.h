#ifndef _FEMUSIC_H_
#define _FEMUSIC_H_

#include "types.h"

namespace FEMusic
{
extern u32 mCurrentFEStreamHash;

void ResetCurrentFEStreamHash();
void StopStream();
void StartStreamIfDifferent(int);
} // namespace FEMusic

#endif // _FEMUSIC_H_

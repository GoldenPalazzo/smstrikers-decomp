#ifndef _MOVIE_H_
#define _MOVIE_H_

#include "NL/nlTask.h"
#include "Game/GameInfo.h"

extern bool g_bActive;

bool IsMoviePlayingInStrikers101();
bool MoviePlay();
bool MovieStop();
bool MovieStart(const char*, bool, bool);
// void 0x8028D534..0x8028D538 | size: 0x4;
#endif // _MOVIE_H_

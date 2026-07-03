#ifndef _MOVIE_H_
#define _MOVIE_H_

#include "NL/nlTask.h"
#include "Game/GameInfo.h"

bool IsMoviePlayingInStrikers101();
bool MoviePlay();
bool MovieStop();
bool MovieStart(const char*, bool, bool);

#endif // _MOVIE_H_

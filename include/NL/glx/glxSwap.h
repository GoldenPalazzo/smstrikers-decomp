#ifndef _GLXSWAP_H_
#define _GLXSWAP_H_

void glxSwapPost(bool);
void glxSwapPre(bool);
void glxInitSwap(void*, void*);
void glxSwapWaitDrawDone();
void glxLoadRestoreState();
void glxLoadSaveState();
void glxSwapLoading(bool bBegin, bool bOtherPosition);
void* glxGetBackBuffer();
void* glxGetDisplayedBuffer();
void glxSwapSetBlack(bool);

#endif // _GLXSWAP_H_

#define NL_SINGLETON_NO_DEFINE
#include "NL/nlSingleton.h"

// Data-only translation unit: the home of the ScriptQuestionCache singleton
// instance pointer and the two question-caching control flags. The
// ScriptQuestionCache class itself (and its Lookup/AddToCache members) lives in
// CommonScript.cpp / ScriptCaching.h; only a forward declaration is needed here
// to define nlSingleton<ScriptQuestionCache>::s_pInstance.
//
//   g_bScriptQuestionCachingOn                     .sdata  0x80371340 (= 1)
//   g_bScriptQuestionCachingUseSTD                 .sbss   0x803737D8
//   nlSingleton<ScriptQuestionCache>::s_pInstance  .sbss   0x803737DC
class ScriptQuestionCache;

unsigned char g_bScriptQuestionCachingOn = 1;
unsigned char g_bScriptQuestionCachingUseSTD;
template <> ScriptQuestionCache* nlSingleton<ScriptQuestionCache>::s_pInstance = 0;

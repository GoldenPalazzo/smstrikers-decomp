#ifndef _GLCONSTANT_H_
#define _GLCONSTANT_H_

#include "NL/nlMath.h"

nlVector4 glConstantGet(const char*);
bool glConstantGet(const char*, nlVector4&);
void glConstantSet(const char*, const nlVector4&);
void gl_ConstantMarkerBackup(int);
void gl_ConstantMarkerAdvance();
void gl_ConstantStartup();

#endif // _GLCONSTANT_H_

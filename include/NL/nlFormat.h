#ifndef _NLFORMAT_H_
#define _NLFORMAT_H_

#include "NL/nlBasicString.h"

template <typename StringType>
class FormatImpl {
public:
    StringType mString;
    int mCurrentPos;

    operator StringType() const {
        return mString;
    }

    template <typename T>
    FormatImpl& operator%(const T& t);
};

#endif // _NLFORMAT_H_

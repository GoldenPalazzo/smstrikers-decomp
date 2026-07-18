#ifndef _COMPRESSOR_H_
#define _COMPRESSOR_H_

template <int MIN, int MAX, int BITS>
class FloatCompressor
{
public:
    FloatCompressor(float& f);
    inline unsigned int Read(LoadFrame& frame) const;
    inline unsigned int Read(SaveFrame& frame) const;
    inline void Transfer(LoadFrame& frame, unsigned int& value) const;
    inline void TransferOR(LoadFrame& frame, unsigned int& value) const;
    inline void Transfer(SaveFrame& frame, unsigned int& value) const;
    inline void TransferOR(SaveFrame& frame, unsigned int& value) const;
    inline void Apply(LoadFrame& frame, unsigned int value) const;
    inline void Apply(SaveFrame& frame, unsigned int value) const;
    inline void Replay(LoadFrame& frame) const;
    inline void Replay(SaveFrame& frame) const;

    /* 0x0 */ float& mF;
}; // total size: 0x4

template <>
FloatCompressor<0, 1, 15>::FloatCompressor(float& f);

template <>
FloatCompressor<0, 1, 7>::FloatCompressor(float& f);

template <int MIN, int MAX, int BITS>
inline FloatCompressor<MIN, MAX, BITS>::FloatCompressor(float& f)
    : mF(f)
{
}

template <int MIN, int MAX, int BITS>
inline unsigned int FloatCompressor<MIN, MAX, BITS>::Read(LoadFrame& frame) const
{
    return 0;
}

template <int MIN, int MAX, int BITS>
inline unsigned int FloatCompressor<MIN, MAX, BITS>::Read(SaveFrame& frame) const
{
    float f = mF;
    if (f > (float)MAX)
        f = (float)MAX;
    if (f < (float)MIN)
        f = (float)MIN;
    f -= (float)MIN;
    f *= (float)(1 << BITS);
    return (unsigned int)f;
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Transfer(LoadFrame& frame, unsigned int& value) const
{
    if ((MAX - MIN) * (1 << BITS) <= 255)
    {
        const char* cursor = frame.mStream.mStorage;
        value = (unsigned int)(unsigned char)*cursor++;
        frame.mStream.mStorage = cursor;
    }
    else if ((MAX - MIN) * (1 << BITS) <= 65535)
    {
        const char* cursor = frame.mStream.mStorage;
        unsigned char lo = (unsigned char)*cursor++;
        value = (unsigned int)lo | ((unsigned int)(unsigned char)*cursor++ << 8);
        frame.mStream.mStorage = cursor;
    }
    else
    {
        const char* cursor = frame.mStream.mStorage;
        unsigned char mid = (unsigned char)cursor[1];
        unsigned char hi = (unsigned char)cursor[2];
        unsigned char lo = (unsigned char)cursor[0];
        cursor += 3;
        frame.mStream.mStorage = cursor;
        value = ((unsigned int)hi << 16) | ((unsigned int)mid << 8) | (unsigned int)lo;
    }
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::TransferOR(LoadFrame& frame, unsigned int& value) const
{
    if ((MAX - MIN) * (1 << BITS) <= 255)
    {
        const char* cursor = frame.mStream.mStorage;
        value = (unsigned int)(unsigned char)*cursor++;
        frame.mStream.mStorage = cursor;
    }
    else if ((MAX - MIN) * (1 << BITS) <= 65535)
    {
        const char* cursor = frame.mStream.mStorage;
        unsigned char lo = (unsigned char)*cursor++;
        unsigned char hi = (unsigned char)*cursor++;
        frame.mStream.mStorage = cursor;
        value |= (unsigned int)hi << 8;
        value |= (unsigned int)lo;
    }
    else
    {
        const char* cursor = frame.mStream.mStorage;
        unsigned char mid = (unsigned char)cursor[1];
        unsigned char hi = (unsigned char)cursor[2];
        unsigned char lo = (unsigned char)cursor[0];
        cursor += 3;
        frame.mStream.mStorage = cursor;
        value = ((unsigned int)hi << 16) | ((unsigned int)mid << 8) | (unsigned int)lo;
    }
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Transfer(SaveFrame& frame, unsigned int& value) const
{
    if ((MAX - MIN) * (1 << BITS) <= 255)
    {
        char* p = frame.mStream.mStorage;
        *p++ = (char)value;
        frame.mStream.mStorage = p;
    }
    else if ((MAX - MIN) * (1 << BITS) <= 65535)
    {
        char* p = frame.mStream.mStorage;
        *p++ = (char)(value & 0xFF);
        *p++ = (char)((value >> 8) & 0xFF);
        frame.mStream.mStorage = p;
    }
    else
    {
        char* p = frame.mStream.mStorage;
        *p++ = (char)(value & 0xFF);
        *p++ = (char)((value >> 8) & 0xFF);
        *p++ = (char)((value >> 16) & 0xFF);
        frame.mStream.mStorage = p;
    }
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::TransferOR(SaveFrame& frame, unsigned int& value) const
{
    if ((MAX - MIN) * (1 << BITS) <= 255)
    {
        char* p = frame.mStream.mStorage;
        *p++ = (char)value;
        frame.mStream.mStorage = p;
    }
    else if ((MAX - MIN) * (1 << BITS) <= 65535)
    {
        char* p = frame.mStream.mStorage;
        *p++ = (char)(value & 0xFF);
        *p++ = (char)((value >> 8) & 0xFF);
        frame.mStream.mStorage = p;
    }
    else
    {
        char* p = frame.mStream.mStorage;
        *p++ = (char)(value & 0xFF);
        *p++ = (char)((value >> 8) & 0xFF);
        *p++ = (char)((value >> 16) & 0xFF);
        frame.mStream.mStorage = p;
    }
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Apply(LoadFrame& frame, unsigned int value) const
{
    mF = (float)value / (float)(1 << BITS);
    mF += (float)MIN;
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Apply(SaveFrame& frame, unsigned int value) const
{
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Replay(LoadFrame& frame) const
{
    unsigned int value = Read(frame);
    Transfer(frame, value);
    Apply(frame, value);
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Replay(SaveFrame& frame) const
{
    unsigned int value = Read(frame);
    Transfer(frame, value);
    Apply(frame, value);
}

#endif // _COMPRESSOR_H_

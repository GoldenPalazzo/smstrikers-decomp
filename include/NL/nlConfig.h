#ifndef _NLCONFIG_H_
#define _NLCONFIG_H_

#include "types.h"
#include "NL/nlLexicalCast.h"
#include "NL/nlBasicString.h"

enum Type
{
    _BOOL = 0,
    _INT = 1,
    _FLOAT = 2,
    _STRING = 3,
};

union Value
{
    /* 0x0 */ const char* s;
    /* 0x0 */ int i;
    /* 0x0 */ bool b;
    /* 0x0 */ float f;
};

class Config
{
public:
    struct TagValuePair
    {
        TagValuePair();

        template <typename T>
        T Get() const
        {
            FORCE_DONT_INLINE;
        }

        /* 0x00 */ const char* tag;
        /* 0x04 */ Type type;
        /* 0x08 */ Value value;
    };

    enum AllocateWhere
    {
        ALLOCATE_LOW = 0,
        ALLOCATE_HIGH = 1,
    };

    struct Parser
    {
        virtual void EmptyLine() { FORCE_DONT_INLINE; }
        virtual void Comment(const BasicString<char, Detail::TempStringAllocator>&) { FORCE_DONT_INLINE; }
        virtual void Section(const BasicString<char, Detail::TempStringAllocator>&) { FORCE_DONT_INLINE; }
        virtual void TagValuePair(const BasicString<char, Detail::TempStringAllocator>&, const BasicString<char, Detail::TempStringAllocator>&) { FORCE_DONT_INLINE; }
    }; // total size: 0x4

    void Parse(const char*, Parser&);
    static BasicString<char, Detail::TempStringAllocator> LoadFileAsString(const char*);
    void Set(const char*, const BasicString<char, Detail::TempStringAllocator>&);
    void Set(const char*, const char*);
    void Set(const char*, float);
    void Set(const char*, bool);
    void Set(const char*, int);

    template <typename T>
    void Set(const char* key, T value);
    TagValuePair& FindTvp(const char*);
    bool IsBool(const char*, bool&) const;
    bool Exists(const char*) const;
    void LoadFromFile(const char*);

    template <typename T>
    T Get(const char* key, T defaultValue)
    {
        FORCE_DONT_INLINE;
        TagValuePair& tvp = FindTvp(key);
        if (tvp.tag == NULL)
        {
            Set(key, defaultValue);
            return defaultValue;
        }

        T val;
        if (tvp.type == _BOOL)
        {
            val = LexicalCast<T, bool>(tvp.value.b);
        }
        else if (tvp.type == _INT)
        {
            val = LexicalCast<T, int>(tvp.value.i);
        }
        else if (tvp.type == _FLOAT)
        {
            val = LexicalCast<T, float>(tvp.value.f);
        }
        else if (tvp.type == _STRING)
        {
            val = LexicalCast<T, const char*>(tvp.value.s);
        }
        else
        {
            val = T();
        }
        return val;
    };

    // BasicString<char, Detail::TempStringAllocator> TagValuePair::Get<BasicString<char, Detail::TempStringAllocator> >() const
    // {
    //     BasicString<char, Detail::TempStringAllocator> result;

    //     if (type == _BOOL)
    //     {
    //         result = LexicalCast<BasicString<char, Detail::TempStringAllocator>, bool>(value.b);
    //     }
    //     else if (type == _INT)
    //     {
    //         result = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(value.i);
    //     }
    //     else if (type == _FLOAT)
    //     {
    //         result = LexicalCast<BasicString<char, Detail::TempStringAllocator>, float>(value.f);
    //     }
    //     else if (type == _STRING)
    //     {
    //         result = LexicalCast<BasicString<char, Detail::TempStringAllocator>, const char*>(value.s);
    //     }
    //     else
    //     {
    //         // Default to empty string for unknown types
    //         result.m_data = nullptr;
    //         result.m_size = 0;
    //         result.m_capacity = 0;
    //         result.m_refCount = 1;
    //     }

    //     return result;
    // }

    static Config& Global();

    ~Config();
    Config() { };
    Config(AllocateWhere);

    /* 0x0 */ TagValuePair* mTvpHash;
    /* 0x4 */ char* mStringMemory;
    /* 0x8 */ char* mStringEnd;

    // Parser::TagValuePair(const BasicString<char, Detail::TempStringAllocator>&, const BasicString<char, Detail::TempStringAllocator>&);
    // Parser::Section(const BasicString<char, Detail::TempStringAllocator>&);
    // Parser::Comment(const BasicString<char, Detail::TempStringAllocator>&);
    // Parser::EmptyLine();
    // TagValuePair::TagValuePair();
    // Set<BasicString<char, Detail::TempStringAllocator>>(const char*, BasicString<char, Detail::TempStringAllocator>);
}; // total size: 0xC

typedef Config::TagValuePair TagValuePair;

struct SetTagValuePair : public Config::Parser
{
    /* 0x04 */ BasicString<char, Detail::TempStringAllocator> mCurrentSection;
    /* 0x08 */ Config& mConfig;
    /* 0x0C */ bool mTweaked;
    /* 0x10 */ float mTweakMinValue;
    /* 0x14 */ float mTweakMaxValue;
    /* 0x18 */ float mTweakIncrement;

    SetTagValuePair(Config& cfg)
        : mConfig(cfg)
        , mTweaked(false)
    {
    }

    virtual void TagValuePair(const BasicString<char, Detail::TempStringAllocator>&, const BasicString<char, Detail::TempStringAllocator>&);
    virtual void Section(const BasicString<char, Detail::TempStringAllocator>&);
    virtual void Comment(const BasicString<char, Detail::TempStringAllocator>&);
}; // total size: 0x1C

// class BasicString<char, Detail
// {
// public:
//     BasicString<char, Detail::TempStringAllocator>::Trim(const char*) const;
//     BasicString<char, Detail::TempStringAllocator>::TrimInPlace(const char*);
//     BasicString<char, Detail::TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<char,
//     Detail::TempStringAllocator>&) const; BasicString<char,
//     Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&);
// };

template <>
inline BasicString<char, Detail::TempStringAllocator> Config::TagValuePair::Get<BasicString<char, Detail::TempStringAllocator> >() const
{
    if (type == _BOOL)
    {
        return LexicalCast<BasicString<char, Detail::TempStringAllocator>, bool>(value.b);
    }
    else if (type == _INT)
    {
        return LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(value.i);
    }
    else if (type == _FLOAT)
    {
        return LexicalCast<BasicString<char, Detail::TempStringAllocator>, float>(value.f);
    }
    else if (type == _STRING)
    {
        return LexicalCast<BasicString<char, Detail::TempStringAllocator>, const char*>(value.s);
    }
    return BasicString<char, Detail::TempStringAllocator>();
}

template <>
inline BasicString<char, Detail::TempStringAllocator> Config::Get<BasicString<char, Detail::TempStringAllocator> >(
    const char* key, BasicString<char, Detail::TempStringAllocator> defaultValue)
{
    TagValuePair& tvp = FindTvp(key);
    if (tvp.tag == NULL)
    {
        Set(key, defaultValue);
        return defaultValue;
    }
    return tvp.Get<BasicString<char, Detail::TempStringAllocator> >();
}

inline float GetConfigFloat(Config& cfg, const char* key, float defaultValue)
{
    TagValuePair& tvp = cfg.FindTvp(key);
    if (tvp.tag == NULL)
    {
        cfg.Set(key, defaultValue);
        return defaultValue;
    }

    float val;
    if (tvp.type == _BOOL)
    {
        val = LexicalCast<float, bool>(tvp.value.b);
    }
    else if (tvp.type == _INT)
    {
        val = LexicalCast<float, int>(tvp.value.i);
    }
    else if (tvp.type == _FLOAT)
    {
        val = LexicalCast<float, float>(tvp.value.f);
    }
    else if (tvp.type == _STRING)
    {
        val = LexicalCast<float, const char*>(tvp.value.s);
    }
    else
    {
        val = 0.f;
    }
    return val;
}

inline s32 GetConfigInt(Config& cfg, const char* key, s32 defaultValue)
{
    TagValuePair& tvp = cfg.FindTvp(key);
    if (tvp.tag == NULL)
    {
        cfg.Set(key, (int)defaultValue);
        return defaultValue;
    }

    int val;
    if (tvp.type == _BOOL)
    {
        val = LexicalCast<int, bool>(tvp.value.b);
    }
    else if (tvp.type == _INT)
    {
        val = LexicalCast<int, int>(tvp.value.i);
    }
    else if (tvp.type == _FLOAT)
    {
        val = LexicalCast<int, float>(tvp.value.f);
    }
    else if (tvp.type == _STRING)
    {
        val = LexicalCast<int, const char*>(tvp.value.s);
    }
    else
    {
        val = 0;
    }
    return val;
}

inline bool GetConfigBool(Config& cfg, const char* key, bool defaultValue)
{
    TagValuePair& tvp = cfg.FindTvp(key);
    bool val;
    if (tvp.tag == NULL)
    {
        cfg.Set(key, defaultValue);
        return defaultValue;
    }

    if (tvp.type == _BOOL)
    {
        val = LexicalCast<bool, bool>(tvp.value.b);
    }
    else if (tvp.type == _INT)
    {
        val = LexicalCast<bool, int>(tvp.value.i);
    }
    else if (tvp.type == _FLOAT)
    {
        val = LexicalCast<bool, float>(tvp.value.f);
    }
    else if (tvp.type == _STRING)
    {
        val = LexicalCast<bool, const char*>(tvp.value.s);
    }
    else
    {
        val = 0.f;
    }
    return val;
}

#endif // _NLCONFIG_H_

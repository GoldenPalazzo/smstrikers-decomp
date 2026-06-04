#ifndef NL_SINGLETON_H
#define NL_SINGLETON_H

template <class T>
class nlSingleton
{
public:
    static T* Instance() { return s_pInstance; }
    static T* GetInstance() { return s_pInstance; }

    static void CreateInstance()
    {
        if (!s_pInstance)
            s_pInstance = new T();
    }

    static void DestroyInstance()
    {
        FORCE_DONT_INLINE;
        T* p = s_pInstance;
        if (p)
        {
            delete p;
            s_pInstance = 0;
        }
    }

    static T* s_pInstance;
};

#ifndef NL_SINGLETON_NO_DEFINE
template <class T>
T* nlSingleton<T>::s_pInstance = 0;
#endif

#endif // NL_SINGLETON_H

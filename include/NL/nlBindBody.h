// Bind template bodies. Include after nlFunction.h.
// Define BIND_NO_DECL before including nlFunction.h so these become the
// declaring definitions (gives Bind<> its own linkonce section in the TU).
template <typename R, typename F, typename A>
BindExp1<R, F, A> Bind(F fn, const A& arg)
{
    return BindExp1<R, F, A>(fn, arg);
}

template <typename R, typename F, typename A, typename B>
BindExp2<R, F, A, B> Bind(F fn, const A& t0, const B& t1)
{
    return BindExp2<R, F, A, B>(fn, t0, t1);
}

#if !BOOST_PP_IS_ITERATING

#ifndef LUA_EVENT_HELPERS_HPP_INCLUDED
#define LUA_EVENT_HELPERS_HPP_INCLUDED LUA_EVENT_HELPERS_HPP_INCLUDED

#include <ssig.hpp>
#include <luabind/object.hpp>
#include <luabind/function.hpp>
#include <boost/bind.hpp>

#include <boost/preprocessor/enum.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum_trailing.hpp>

#define LUA_EVENT_HELPERS_MAX_ARGS SSIG_MAX_ARGS

#define JD_EVENT_ENTRY(ename, r, ...) \
    if (name == BOOST_STRINGIZE(ename))           \
        return makeConnection(cc->connect_##ename(boost::bind( \
            LuaFunction<r>(recv), __VA_ARGS__))); \
    else { }

#define JD_EVENT_ENTRY0(ename, r) \
    if (name == BOOST_STRINGIZE(ename))          \
        return makeConnection(cc->connect_##ename(boost::bind( \
            &luabind::call_function<r>, recv))); \
    else { }

#define JD_EVENT_TABLE_BEGIN(ct) \
    ConnectionBase* ct##Meta::connectEvent( \
            lua_State* L,                   \
            Component* c,                   \
            std::string const& name) const  \
    {                                       \
        ct* cc = c->as<ct>();               \
        luabind::object recv(luabind::from_stack(L, -1)); \
        using boost::ref; using boost::cref;

#define JD_EVENT_TABLE_END return nullptr; }


template<typename Signature>
ScopedConnection<Signature>* makeConnection(Connection<Signature> const& con)
{
    return new ScopedConnection<Signature>(con);
}

template <typename R>
struct LuaFunction
{
    typedef R result_type;

	LuaFunction(luabind::object const& obj)
		: m_func(obj)
	{
	}

    R operator() ()
    {
        return luabind::call_function<R>(m_func);
    }

#define BOOST_PP_ITERATION_LIMITS (1, LUA_EVENT_HELPERS_MAX_ARGS)
#define BOOST_PP_FILENAME_1 "LuaEventHelpers.hpp" // include self
#include BOOST_PP_ITERATE()

    private:
	luabind::object m_func;
};

template <>
struct LuaFunction<void>
{
    typedef void result_type;

	LuaFunction(luabind::object const& obj)
		: m_func(obj)
	{
	}

    void operator() ()
    {
        luabind::call_function<void>(m_func);
    }

#define VOID_SPEC
#define BOOST_PP_ITERATION_LIMITS (1, LUA_EVENT_HELPERS_MAX_ARGS)
#define BOOST_PP_FILENAME_1 "LuaEventHelpers.hpp" // include self
#include BOOST_PP_ITERATE()
#undef VOID_SPEC

    private:
	luabind::object m_func;
};


#endif // include guard

#else  // !BOOST_PP_IS_ITERATING

#define NARGS BOOST_PP_ITERATION()

#define TMPL_PARAMS                BOOST_PP_ENUM_PARAMS(NARGS, typename A)
#define PRINT_TYPED_ARG_R(z, n, _) BOOST_PP_CAT(A, n)&& BOOST_PP_CAT(arg, n)
#define TYPED_ARGS_RREF            BOOST_PP_ENUM(NARGS, PRINT_TYPED_ARG_R, BOOST_PP_EMPTY())
#define PRINT_FORWARD_ARG(z, n, _) std::forward<BOOST_PP_CAT(A, n)>(BOOST_PP_CAT(arg, n))
#define FORWARD_ARGS               BOOST_PP_ENUM_TRAILING(NARGS, PRINT_FORWARD_ARG, BOOST_PP_EMPTY())

#ifdef VOID_SPEC
    template <TMPL_PARAMS>
    void operator() (TYPED_ARGS_RREF)
    {
        luabind::call_function<void>(m_func FORWARD_ARGS);
    }
#else
    template <TMPL_PARAMS>
    R operator() (TYPED_ARGS_RREF)
    {
        return luabind::call_function<R>(m_func FORWARD_ARGS);
    }
#endif

#undef TMPL_PARAMS
#undef PRINT_TYPED_ARG_R
#undef TYPED_ARGS_RREF
#undef PRINT_FORWARD_ARG
#undef FORWARD_ARGS
#undef NARGS

#endif // BOOST_PP_IS_ITERATING
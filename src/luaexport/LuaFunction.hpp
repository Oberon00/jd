#if !BOOST_PP_IS_ITERATING

#ifndef LUA_FUNCTION_HPP_INCLUDED
#define LUA_FUNCTION_HPP_INCLUDED LUA_FUNCTION_HPP_INCLUDED

#include <boost/preprocessor/enum.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum_trailing.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <luabind/function.hpp>
#include <luabind/object.hpp>


#ifndef LUA_FUNCTION_MAX_ARGS
#   define LUA_FUNCTION_MAX_ARGS 5
#endif

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

#define BOOST_PP_ITERATION_LIMITS (1, LUA_FUNCTION_MAX_ARGS)
#define BOOST_PP_FILENAME_1 "luaexport/LuaFunction.hpp" // include self
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
#define BOOST_PP_ITERATION_LIMITS (1, LUA_FUNCTION_MAX_ARGS)
#define BOOST_PP_FILENAME_1 "luaexport/LuaFunction.hpp" // include self
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

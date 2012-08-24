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

#define BOOST_PP_ITERATION_LIMITS (0, LUA_EVENT_HELPERS_MAX_ARGS)
#define BOOST_PP_FILENAME_1 "LuaEventHelpers.hpp" // include self
#include BOOST_PP_ITERATE()

#endif // include guard

#else  // !BOOST_PP_IS_ITERATING

#define NARGS BOOST_PP_ITERATION()

#define TYPES                      BOOST_PP_ENUM_PARAMS(NARGS, A)
#define TMPL_PARAMS                BOOST_PP_ENUM_TRAILING_PARAMS(NARGS, typename A)
//#define PRINT_TYPED_ARG_R(z, n, _) BOOST_PP_CAT(A, n)&& BOOST_PP_CAT(arg, n)
//#define TYPED_ARGS_RREF            BOOST_PP_ENUM_TRAILING(NARGS, PRINT_TYPED_ARG_R, BOOST_PP_EMPTY())
#define PRINT_TYPED_ARG(z, n, _)   BOOST_PP_CAT(A, n) BOOST_PP_CAT(arg, n)
#define TYPED_ARGS                 BOOST_PP_ENUM_TRAILING(NARGS, PRINT_TYPED_ARG, BOOST_PP_EMPTY())

//#define PRINT_FORWARD_ARG(z, n, _) std::forward<BOOST_PP_CAT(A, n)>(BOOST_PP_CAT(arg, n))
//#define FORWARD_ARGS               BOOST_PP_ENUM_TRAILING(NARGS, PRINT_FORWARD_ARG, BOOST_PP_EMPTY())
#define ARGS BOOST_PP_ENUM_TRAILING_PARAMS(NARGS, arg)
#define PRINT_FORWARD_ARG_D(z, n, _) decltype(std::forward<BOOST_PP_CAT(A, n)>(BOOST_PP_CAT(arg, n)))
#define FORWARD_ARGS_D             BOOST_PP_ENUM_TRAILING(NARGS, PRINT_FORWARD_ARG_D, BOOST_PP_EMPTY())

#define EXPR \
    boost::bind(                                \
        (                                       \
            decltype (                          \
                luabind::call_function<R>(      \
                    obj ARGS            \
                )                               \
            )(*)(                               \
                luabind::object const& BOOST_PP_COMMA_IF(NARGS) TYPES \
            )                                   \
            )&luabind::call_function<R>, obj ARGS \
    )

template <typename R TMPL_PARAMS>
boost::function<R(TYPES)> bindLuaObject(luabind::object const& obj TYPED_ARGS)
{
    return EXPR;
}

#undef EXPR
#undef TYPES
#undef TMPL_PARAMS
#undef PRINT_TYPED_ARG
#undef PRINT_TYPED_ARG_R
#undef TYPED_ARGS
#undef TYPED_ARGS_RREF
#undef PRINT_FORWARD_ARG
#undef FORWARD_ARGS


#endif // BOOST_PP_IS_ITERATING
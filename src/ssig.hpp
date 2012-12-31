#ifndef SSIG_HPP_INCLUDED
#define SSIG_HPP_INCLUDED SSIG_HPP_INCLUDED

#ifndef SSIG_MAX_ARGS
#	define SSIG_MAX_ARGS 5
#endif

#include <boost/function.hpp>
#include <list>
#include <type_traits>
#include <boost/next_prior.hpp>
#include <boost/noncopyable.hpp>
#include <boost/preprocessor/enum.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum_trailing.hpp>


class SsigError: public std::logic_error
{
public:
	SsigError(char const* msg): std::logic_error(msg) { }
	SsigError(std::string const& msg): std::logic_error(msg) { }
};

namespace detail {

template<typename R>
inline R default_result()
{
	throw SsigError("attempt to invoke empty signal with non-void return type");
}

template<>
inline void default_result<void>()
{ }

template <typename R>
inline bool check(bool isLast) { return isLast; }

template <>
inline bool check<void>(bool) { return false; }

} // namespace detail

class ConnectionBase
{
public:
    virtual ~ConnectionBase() {}
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
};


template<class Signature>
class Signal;

template<class Signature>
class Connection;

template<class Signature>
class ScopedConnection;

#define BOOST_PP_ITERATION_LIMITS (0, SSIG_MAX_ARGS)
#define BOOST_PP_FILENAME_1 "ssig_template.hpp"
#include BOOST_PP_ITERATE()

#undef NARGS
#undef TYPES
#undef TRAILING_TMPL_PARAMS
#undef PRINT_TYPED_ARG
#undef PRINT_FORWARD_ARG
#undef TYPED_ARGS
#undef ARGS

#define SSIG_DEFINE_MEMBERSIGNAL(name, signature) \
    public:                                                               \
        Signal<signature>::connection_type const connect_##name(          \
            Signal<signature>::function_type const& slot)                 \
        {                                                                 \
            return m_sig_##name.connect(slot);                            \
        }                                                                 \
    private: Signal<signature> m_sig_##name;

    
#define SSIG_DEFINE_STATICSIGNAL(name, signature) \
    public:                                                               \
        static Signal<signature>::connection_type const connect_##name(   \
            Signal<signature>::function_type const& slot)                 \
        {                                                                 \
            return sig_##name().connect(slot);                            \
        }                                                                 \
    private:                                                              \
        static Signal<signature>& sig_##name()                            \
        {                                                                 \
            static Signal<signature> sig;                                 \
            return sig;                                                   \
        }

#endif

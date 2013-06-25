// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#if !BOOST_PP_IS_ITERATING
#   error "Do not include this file! Include ssig.hpp instead."
#endif

#define NARGS BOOST_PP_ITERATION()

#define TYPES BOOST_PP_ENUM_PARAMS(NARGS, A)
#define TRAILING_TMPL_PARAMS BOOST_PP_ENUM_TRAILING_PARAMS(NARGS, typename A)

#define PRINT_TYPED_ARG(z, n, _) BOOST_PP_CAT(A, n) BOOST_PP_CAT(arg, n)
#define TYPED_ARGS BOOST_PP_ENUM(NARGS, PRINT_TYPED_ARG, BOOST_PP_EMPTY())
#define PRINT_TYPED_ARG_RREF(z, n, _) BOOST_PP_CAT(A, n)&& BOOST_PP_CAT(arg, n)
#define TRAILING_TYPED_ARGS_RREF BOOST_PP_ENUM_TRAILING(NARGS, PRINT_TYPED_ARG_RREF, BOOST_PP_EMPTY())
#define PRINT_FORWARD_ARG(z, n, _) std::forward<BOOST_PP_CAT(A, n)>(BOOST_PP_CAT(arg, n))
#define ARGS BOOST_PP_ENUM(NARGS, PRINT_FORWARD_ARG, BOOST_PP_EMPTY())

template<typename F TRAILING_TMPL_PARAMS>
void discard_result(F f TRAILING_TYPED_ARGS_RREF)
{
    f(ARGS);
}

template<typename R TRAILING_TMPL_PARAMS>
class Signal<R(TYPES)> {
public:
    typedef boost::function<R(TYPES)> function_type;
    typedef Connection<R(TYPES)> connection_type;

    R const operator() (TYPED_ARGS)
    {
        for (auto it = m_slots.begin(); it != m_slots.end(); ) {
            if (!it->slot) {
                it = m_slots.erase(it);
            } else {
                if (detail::check<R>(boost::next(it) == m_slots.end()))
                    return it->slot(ARGS); // return last result
                it->slot(ARGS); // discard all other results
                ++it;
            }
        }
        return detail::default_result<R>();
    }


    connection_type connect(function_type const& slot);

    bool empty() const { return m_slots.empty(); }


private:
    friend Connection<R(TYPES)>;

    typedef std::list<connection_type*> connection_container_type;
    struct SlotRef: private boost::noncopyable {
        SlotRef(function_type const& slot, connection_type* first_connection):
            slot(slot) { connections.push_back(first_connection); }
        SlotRef(SlotRef&& rhs):
            connections(std::move(rhs.connections)),
            slot(std::move(rhs.slot))
        {
        }
        ~SlotRef();
        connection_container_type connections;
        function_type slot;
    };

    typedef std::list<SlotRef> container_type;
    container_type m_slots;


};

template<typename R TRAILING_TMPL_PARAMS>
class Connection<R(TYPES)>: public ConnectionBase {
public:
    typedef Signal<R(TYPES)> signal_type;
    static_assert(
        std::is_same<typename signal_type::connection_type, Connection>::value,
        "internal error: inconsistent typedef");

    Connection(): m_signal(nullptr) { } // construct disconnected signal

    Connection(signal_type& signal, typename signal_type::function_type const& slot):
        m_signal(&signal)
    {
        signal.m_slots.push_front(std::move(
            typename Signal<R(TYPES)>::SlotRef(slot, this)));
        m_iterator = signal.m_slots.begin();
    }


    ~Connection()
    {
        if (m_signal)
            m_iterator->connections.remove(this);
    }

    Connection(Connection const& rhs)
    {
        copyFrom(rhs);
    }

    Connection(Connection&& rhs)
    {
        takeOver(std::move(rhs));
    }

    Connection& operator= (Connection const& rhs)
    {
        if (isConnected())
            disconnect();
        copyFrom(rhs);
        return *this;
    }

    Connection& operator= (Connection&& rhs)
    {
        if (isConnected())
            disconnect();
        takeOver(std::move(rhs));
        return *this;
    }

    void disconnect()
    {
        checkSignal();
        m_iterator->slot.clear();
        m_iterator->connections.remove(this);
        m_signal = nullptr;
    }

    signal_type& signal() { checkSignal(); return *m_signal; }

    bool isConnected() const { return m_signal != nullptr; };

    R invokeSlot(TYPED_ARGS) { return m_iterator->slot(ARGS); }

private:
    void takeOver(Connection&& rhs)
    {
        m_signal = rhs.m_signal;
        rhs.m_signal = nullptr;
        if (m_signal) {
           m_iterator = rhs.m_iterator;
            auto it = std::find(
                m_iterator->connections.begin(),
                m_iterator->connections.end(), &rhs);
            assert(it != m_iterator->connections.end());
            *it = this;
        }
    }

    void copyFrom(Connection const& rhs)
    {
        m_signal = rhs.m_signal;
        if (m_signal) {
            m_iterator = rhs.m_iterator;
            m_iterator->connections.push_back(this);
        }
    }

    friend Signal<R(TYPES)>;
    void signalDestroyed() { m_signal = nullptr; }

    void checkSignal() const
    {
        if (!m_signal)
            throw SsigError("attempt to use a disconnected signal");
    }

    signal_type* m_signal;
    typename signal_type::container_type::iterator m_iterator;
};

template<typename R TRAILING_TMPL_PARAMS>
typename Signal<R(TYPES)>::connection_type Signal<R(TYPES)>::connect(function_type const& slot)
{
    return connection_type(*this, slot);
}

template<typename R TRAILING_TMPL_PARAMS>
Signal<R(TYPES)>::SlotRef::~SlotRef()
{
    for (auto it = connections.begin(); it != connections.end(); ++it) {
        (*it)->signalDestroyed();
    }
}

template<typename R TRAILING_TMPL_PARAMS>
class ScopedConnection<R(TYPES)>: public Connection<R(TYPES)>, private boost::noncopyable
{
    typedef Connection<R(TYPES)> base_t;
    typedef typename base_t::signal_type signal_type;
public:
    ScopedConnection() { }

    ScopedConnection(signal_type& signal, typename signal_type::function_type const& slot):
      base_t(signal, slot)
    { }

    ScopedConnection(base_t const& rhs):
        base_t(rhs)
    { }

    ScopedConnection(ScopedConnection&& rhs):
        base_t(std::move(rhs))
    { }

    ~ScopedConnection()
    {
        if (this->isConnected())
            this->disconnect();
    }

    ScopedConnection& operator= (base_t const& rhs)
    {
        base_t::operator=(rhs);
        return *this;
    }

    ScopedConnection& operator= (base_t&& rhs)
    {
        base_t::operator=(std::forward<base_t&&>(rhs));
        return *this;
    }

private:
    ScopedConnection& operator= (ScopedConnection const&);
};

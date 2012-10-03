#ifndef WEAK_REF_HPP_INCLUDED
#define WEAK_REF_HPP_INCLUDED WEAK_REF_HPP_INCLUDED

#include <boost/noncopyable.hpp>
#include <cassert>
#include <stdexcept>
#include <type_traits>

class InvalidWeakReferenceError: public std::logic_error
{
public:
    InvalidWeakReferenceError(): std::logic_error("attempt to use an invalid WeakReference") { }
};

template <typename T>
class EnableWeakRefFromThis;

namespace detail {
struct WeakRefConnection: private boost::noncopyable {
    WeakRefConnection(void* r): referenced(r), refCount(0) { }
    
    void unref() {
        assert(refCount > 0);
        if (--refCount == 0 && !referenced)
            delete this;
    }

    void invalidate() {
        assert(referenced);
        referenced = nullptr;
        if (refCount == 0)
            delete this;
    }

    unsigned refCount;
    void* referenced;
};

template<typename T>
::detail::WeakRefConnection* getConnection(EnableWeakRefFromThis<T>* r);

} // namespace detail

template <typename T>
class WeakRef {
public:

    WeakRef():
        m_offset(0),
        m_connection(new ::detail::WeakRefConnection(nullptr))
    {
        ++m_connection->refCount;
    }

    WeakRef(EnableWeakRefFromThis<T>* t);


    template<typename U>
    WeakRef(EnableWeakRefFromThis<U>* u);

    WeakRef(WeakRef const& rhs):
        m_offset(0),
        m_connection(rhs.m_connection)
    {
        ++m_connection->refCount;
    }

    WeakRef& operator= (WeakRef const& rhs)
    {
        m_connection->unref();
        m_connection = rhs.m_connection;
        ++m_connection->refCount;
        return *this;
    }
    
    ~WeakRef()
    {
        m_connection->unref();
    }

    bool operator== (WeakRef const& rhs) const { return rhs.getOpt() == getOpt(); }

    bool operator!= (WeakRef const& rhs) const { return rhs.getOpt() != getOpt(); }

    T& operator* () const { return *validate(); }
    T* operator-> () const { return validate(); }
    T* get() const { return validate(); }
    T* getOpt() const {  return m_connection->referenced ? deref() : nullptr; }
    
    bool operator! () const { return !valid(); }
    bool valid() const { return m_connection->referenced != nullptr; }

private:
    T* deref() const
    {
        return reinterpret_cast<T*>(
            reinterpret_cast<intptr_t>(m_connection->referenced) + m_offset);
    }

    T* validate() const
    {
        if (!m_connection->referenced)
            throw InvalidWeakReferenceError();
        return deref();
    }

    std::ptrdiff_t m_offset;
    ::detail::WeakRefConnection* m_connection;
};

template <typename T>
T* get_pointer(WeakRef<T>& r)
{
    return r.get();
}

template <typename T>
T const* get_pointer(WeakRef<T> const& r)
{
    return r.get();
}

template <typename T>
class EnableWeakRefFromThis {
public:
    typedef T referenced_type;

    EnableWeakRefFromThis():
      m_connection(nullptr)
    { }

    EnableWeakRefFromThis(EnableWeakRefFromThis const& rhs):
      m_connection(nullptr) // reset connection
    { }

    WeakRef<T> ref()
    {
        ensureConnection();
        return WeakRef<T>(this);
    }


    template<typename U>
    WeakRef<U> ref()
    {
        return WeakRef<U>(this);
    }

    ~EnableWeakRefFromThis()
    {
        if (m_connection)
            m_connection->invalidate();
    }

private:
    friend ::detail::WeakRefConnection* ::detail::getConnection(EnableWeakRefFromThis<T>* r);

    void ensureConnection()
    {
        if (!m_connection)
            m_connection = new ::detail::WeakRefConnection(static_cast<T*>(this));
    }

    ::detail::WeakRefConnection* m_connection;
};

template <typename T>
bool isValidWeakRef(WeakRef<T> ref)
{
    return ref.valid();
}

namespace detail {
template <typename T>
::detail::WeakRefConnection* getConnection(EnableWeakRefFromThis<T>* r)
{
    if (!r)
        return new WeakRefConnection(nullptr);
    r->ensureConnection();
    return r->m_connection;
}
} // namespace detail

template<typename T>
template<typename U>
WeakRef<T>::WeakRef(EnableWeakRefFromThis<U>* u)
{
    static_assert(std::is_convertible<T*, U*>::value, "Incompatible pointers!");
    m_connection = ::detail::getConnection(u);
    assert(m_connection->referenced == static_cast<U*>(u));
    T* t = static_cast<T*>(static_cast<U*>(u));
    m_offset = (intptr_t)t - (intptr_t)m_connection->referenced;
    if (u)
        assert(dynamic_cast<T*>(static_cast<U*>(u)));
    ++m_connection->refCount;
}

template<typename T>
WeakRef<T>::WeakRef(EnableWeakRefFromThis<T>* t):
    m_offset(0),
    m_connection(::detail::getConnection(t))   
{
    assert(m_connection->referenced == static_cast<T*>(t));
    ++m_connection->refCount;
}


#endif

#ifndef RESFWD_HPP_INCLUDED
#define RESFWD_HPP_INCLUDED RESFWD_HPP_INCLUDED

#include <memory>
#include <string>
#include "exceptions.hpp"

template<typename ResT>
class ResourceManager;

template<typename ResT>
struct ResourceTraits {
    typedef ResourceTraits<ResT> Self;
    typedef ResT Resource;

    typedef ResourceManager<ResT> Manager;
    typedef std::shared_ptr<Resource> Ptr;
    typedef std::shared_ptr<Resource const> ConstPtr;
    typedef std::weak_ptr<Resource> WeakPtr;
    typedef std::weak_ptr<Resource const> ConstWeakPtr;
};

#endif

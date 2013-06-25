// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef RESFWD_HPP_INCLUDED
#define RESFWD_HPP_INCLUDED RESFWD_HPP_INCLUDED

#include "exceptions.hpp"

#include <memory>
#include <string>


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

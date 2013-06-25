// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef AUTO_RESOURCE_HPP_INCLUDED
#define AUTO_RESOURCE_HPP_INCLUDED AUTO_RESOURCE_HPP_INCLUDED

#include "resfwd.hpp"


// Provide a function of the following form to use AutoResource<mediaT, resT>:
//
// void updateAutoResourceMedia(mediaT& m, typename ResourceTraits<resT>::Ptr p)
// {
//    m.setWhatever(*p);
// }

template <typename mediaT, typename resT>
class AutoResource: public mediaT
{
public:
    typedef typename ResourceTraits<resT>::Ptr Ptr;
    typedef typename ResourceTraits<resT>::ConstPtr ConstPtr;

    explicit AutoResource(Ptr resource);
    AutoResource(){};
    virtual ~AutoResource(){};
    ConstPtr resource() const;
    Ptr resource();
    void setResource(Ptr resource);

private:
    Ptr m_resource;
};

template <typename mediaT, typename resT>
AutoResource<mediaT, resT>::AutoResource(Ptr resource):
    mediaT(*resource),
    m_resource(std::move(resource))
{}

template <typename mediaT, typename resT>
typename AutoResource<mediaT, resT>::ConstPtr
AutoResource<mediaT, resT>::resource() const
{
    return m_resource;
}

template <typename mediaT, typename resT>
typename AutoResource<mediaT, resT>::Ptr
AutoResource<mediaT, resT>::resource()
{
    return m_resource;
}

template <typename mediaT, typename resT>
void AutoResource<mediaT, resT>::setResource(
    typename AutoResource<mediaT, resT>::Ptr resource)
{
    m_resource = std::move(resource);
    updateAutoResourceMedia(*this, m_resource);
}


#endif

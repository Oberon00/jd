// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef TRANSFORM_GROUP_HPP_INCLUDED
#define TRANSFORM_GROUP_HPP_INCLUDED TRANSFORM_GROUP_HPP_INCLUDED

#include "exceptions.hpp"
#include "WeakRef.hpp"

#include <boost/noncopyable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include <list>

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable:4355) // 'this' : used in base member initializer list
#endif


class TransformGroup:
    public sf::Drawable, public sf::Transformable,
    public EnableWeakRefFromThis<TransformGroup>,
    // Copies would have to notify all owners of Item*s because if
    // they delete the sf::Drawables, all TransformGroups which
    // have a pointer to it need to know.
    public boost::noncopyable
{
public:
    struct Item {
        Item(sf::Drawable* drawable, bool visible = true):
            drawable(drawable), visible(visible) { }

        Item(Item&& rhs):
           drawable(rhs.drawable), visible(rhs.visible)
        { rhs.drawable = nullptr; }

        Item& operator= (Item&& rhs)
        {
            drawable = rhs.drawable;
            visible = rhs.visible;
            rhs.drawable = nullptr;
            return *this;
        }

        sf::Drawable* drawable;
        bool visible;
    };

    class AutoEntry: private boost::noncopyable {
    public:
        AutoEntry(TransformGroup& g, sf::Drawable* d, bool visible = true);

        AutoEntry();

        ~AutoEntry();

        void setDrawable(sf::Drawable* d);

        sf::Drawable* drawable();

        void release();

        WeakRef<TransformGroup> group() const;
        void setGroup(WeakRef<TransformGroup> g);

        bool visible() const;
        void setVisible(bool visible);

    private:
        WeakRef<TransformGroup> m_group;
        Item* m_entry;
    };


    TransformGroup();
    TransformGroup(TransformGroup&& rhs);
    TransformGroup& operator= (TransformGroup&& rhs);

    Item* add(sf::Drawable* drawable, bool visible = true);

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
private:
    void doDraw(sf::RenderTarget& target, sf::RenderStates states);
    std::list<Item> m_items;
    TransformGroup& operator= (TransformGroup const&);
};

template <typename drawableT>
class GroupedDrawable: public drawableT, public TransformGroup::AutoEntry {
public:
    GroupedDrawable() { }

    explicit GroupedDrawable(TransformGroup& g, bool visible = true):
        AutoEntry(g, this, visible) { }

    GroupedDrawable(GroupedDrawable const& rhs):
        drawableT(rhs)
    {
        setGroup(rhs.group().getOpt());
    }

    GroupedDrawable& operator=(GroupedDrawable const& rhs)
    {
        drawableT::operator=(rhs);
        setGroup(rhs.group());
        return *this;
    }

    void setGroup(TransformGroup* g)
    {
        AutoEntry::setGroup(g);
        if (g)
            setDrawable(this);
    }

};

#ifdef _MSC_VER
#   pragma warning(pop)
#endif

#endif

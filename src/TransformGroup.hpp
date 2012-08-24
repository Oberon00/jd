#ifndef TRANSFORM_GROUP_HPP_INCLUDED
#define TRANSFORM_GROUP_HPP_INCLUDED TRANSFORM_GROUP_HPP_INCLUDED

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <boost/noncopyable.hpp>
#include <list>
#include "WeakRef.hpp"
#include "exceptions.hpp"

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable:4355)
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
    struct Item: private boost::noncopyable {
        Item(sf::Drawable* drawable, bool visible = true):
            drawable(drawable), visible(visible) { }
        sf::Drawable* drawable;
        bool visible;

        Item(Item&& rhs):
           drawable(rhs.drawable), visible(rhs.visible)
        { rhs.drawable = nullptr; }
    };

    class AutoEntry: private boost::noncopyable {
    public:
        AutoEntry(TransformGroup& g, sf::Drawable* d, bool visible = true):
          m_group(g.ref()),
          m_entry(g.add(d, visible))
        { }
        AutoEntry(): m_entry(nullptr) { }
        ~AutoEntry() { release(); }
        void release()
        {
            if (m_entry && m_group.valid())
                m_entry->drawable = nullptr;
        }

        WeakRef<TransformGroup> group() const { return m_group; }
        void setGroup(TransformGroup& g) { m_group = g.ref(); }

        bool visible() const { return m_entry && m_entry->visible; }
        void setVisible(bool visible) {
            if (!m_entry)
                throw jd::Exception("attemt to set visibility of NULL-AutoEntry");
            m_entry->visible = visible;
        }
        
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
};

#ifdef _MSC_VER
#   pragma warning(pop)
#endif

#endif
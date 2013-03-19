#include "TransformGroup.hpp"

#include <SFML/Graphics/RenderTarget.hpp>


TransformGroup::TransformGroup()
{
}

TransformGroup::TransformGroup(TransformGroup&& rhs)
{
    m_items.swap(rhs.m_items);
}

TransformGroup& TransformGroup::operator= (TransformGroup&& rhs)
{
    m_items = std::move(rhs.m_items);
    return *this;
}

TransformGroup::Item* TransformGroup::add(
    sf::Drawable* drawable,
    bool visible)
{
    m_items.push_back(Item(drawable, visible));
    return &m_items.back();
}

void TransformGroup::draw(
    sf::RenderTarget& target,
    sf::RenderStates states)
const
{
    const_cast<TransformGroup*>(this)->doDraw(target, states);
}

void TransformGroup::doDraw(sf::RenderTarget& target, sf::RenderStates states)
{
    states.transform *= getTransform();
    for (auto it = m_items.begin(); it != m_items.end(); ) {
        if (it->drawable) {
            if (it->visible)
                target.draw(*it->drawable);
            ++it;
        } else {
            it = m_items.erase(it);
        }
    }
}


TransformGroup::AutoEntry::AutoEntry(
    TransformGroup& g, sf::Drawable* d, bool visible):
    m_group(g.ref()),
    m_entry(g.add(d, visible))
{ }

TransformGroup::AutoEntry::AutoEntry():
    m_entry(nullptr)
{ }

TransformGroup::AutoEntry::~AutoEntry()
{
    release();
}

void TransformGroup::AutoEntry::setDrawable(sf::Drawable* d)
{
    if (d) {
        if (!m_group.valid())
            throw jd::Exception("attempt to set drawable of NULL-AutoEntry");
        if (m_entry)
            m_entry->drawable = d;
        else
            m_entry = m_group->add(d);
    } else {
        release();
    }
}

sf::Drawable* TransformGroup::AutoEntry::drawable()
{
    return m_entry && m_group.valid() ? m_entry->drawable : nullptr;
}

void TransformGroup::AutoEntry::release()
{
    if (m_entry && m_group.valid()) {
        m_entry->drawable = nullptr;
        m_entry = nullptr;
    }
}

WeakRef<TransformGroup> TransformGroup::AutoEntry::group() const
{
    return m_group;
}

void TransformGroup::AutoEntry::setGroup(WeakRef<TransformGroup> g)
{
    if (!g) {
        release();
        return;
    }

    Item entry(nullptr);
    if (m_group.valid() && m_entry) {
        entry.drawable = m_entry->drawable;
        entry.visible = m_entry->visible;
        release();
    }
    m_group = g;
    if (entry.drawable)
        m_entry = m_group->add(entry.drawable, entry.visible);
    else
        m_entry = nullptr;
}

bool TransformGroup::AutoEntry::visible() const
{
    return m_group.valid() && m_entry && m_entry->visible;
}

void TransformGroup::AutoEntry::setVisible(bool visible)
{
    if (!m_entry || !m_group)
        throw jd::Exception("attempt to set visibility of NULL-AutoEntry");
    m_entry->visible = visible;
}

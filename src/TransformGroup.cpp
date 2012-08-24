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

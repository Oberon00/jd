// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef POSITION_COMPONENT_HPP_INCLUDED
#define POSITION_COMPONENT_HPP_INCLUDED POSITION_COMPONENT_HPP_INCLUDED

#include "compsys/Component.hpp"

#include "ssig.hpp"

#include <SFML/Graphics/Rect.hpp>


class Entity;

class PositionComponent: public Component {
    JD_COMPONENT

    SSIG_DEFINE_MEMBERSIGNAL(rectChanged,
        void(sf::FloatRect const&, sf::FloatRect const&))
public:
    PositionComponent() { }
    explicit PositionComponent(Entity& parent);

    sf::FloatRect rect() const { return m_rect; }
    void setRect(sf::FloatRect const& r);

    sf::Vector2f size() const;
    void setSize(sf::Vector2f sz);

    sf::Vector2f position() const;
    void setPosition(sf::Vector2f p);
    void move(sf::Vector2f d);

private:
    sf::FloatRect m_rect;
};

#endif

// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef RECTCOLLISIONCOMPONENT_HPP_INCLUDED
#define RECTCOLLISIONCOMPONENT_HPP_INCLUDED RECTCOLLISIONCOMPONENT_HPP_INCLUDED

#include "compsys/Component.hpp"

#include "compsys/Entity.hpp"
#include "ssig.hpp"

#include <SFML/Graphics/Rect.hpp>

#include <string>


class RectCollisionComponent: public Component {
    JD_COMPONENT
    SSIG_DEFINE_MEMBERSIGNAL(collided, void(sf::FloatRect, Entity&, sf::FloatRect))

public:
    RectCollisionComponent() { }
    explicit RectCollisionComponent(Entity& parent) { parent.add(*this); }

    void notifyCollision(sf::FloatRect rect, Entity& other, sf::FloatRect otherRect)
    {
        m_sig_collided(rect, other, otherRect);
    }
};

#endif

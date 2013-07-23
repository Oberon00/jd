// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "sfUtil.hpp"

#include <boost/lexical_cast.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Text.hpp>

#include <cwctype>


namespace jd {

std::string const keyName(int keyCode)
{
    switch(keyCode) {
#   define KEY(n, v) case sf::Keyboard::v: return n;
#   include "sfKeyCodes.hpp"
    }
    return "key #" + boost::lexical_cast<std::string>(keyCode);
}

sf::FloatRect viewRect(sf::View const& view)
{
    sf::Vector2f const viewCenter(view.getCenter());
    sf::Vector2f const viewSize(view.getSize());
    sf::FloatRect const viewRect(
        viewCenter.x - viewSize.x / 2,
        viewCenter.y - viewSize.y / 2,
        viewSize.x,
        viewSize.y);
    return viewRect;
}

void setViewRect(sf::View& view, sf::FloatRect const& newRect)
{
    view.setCenter(center(newRect));
    view.setSize(newRect.width, newRect.height);
}

void breakTextLines(sf::Text& t, float maxX)
{
    sf::String s = t.getString();
    std::size_t lastBreakCharIdx = s.getSize();
    static sf::String const breakBeforeChars("([{\"'`'");
    static auto const isBreakBeforeChar = [] (sf::Uint32 c) {
        return breakBeforeChars.find(c) != sf::String::InvalidPos;
    };
    for (std::size_t i = 0; i < s.getSize(); ++i) {
        if (t.findCharacterPos(i).x > maxX) {
            if (lastBreakCharIdx > i)
                lastBreakCharIdx = i;
            if (s.getSize() > lastBreakCharIdx &&
                !std::iswgraph(static_cast<std::wint_t>(s[lastBreakCharIdx + 1]))
            ) {
                s[lastBreakCharIdx + 1] = '\n';
            } else {
                s.insert(lastBreakCharIdx + 1, '\n');
            }
            t.setString(s);
            i += 1;
        }
        if (!std::iswalnum(static_cast<std::wint_t>(s[i]))) {
            lastBreakCharIdx = i;
            if (i > 0 && isBreakBeforeChar(s[i]))
                lastBreakCharIdx -= 1;
        }
    }
}

} // namespace jd

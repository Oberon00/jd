#include "sfUtil.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <boost/lexical_cast.hpp>

namespace jd {

std::string const keyName(int keyCode)
{
    switch(keyCode) {
#   define E(n, v) case sf::Keyboard::v: return n;
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

} // namespace jd

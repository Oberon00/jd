#include "DrawService.hpp"
#include <boost/foreach.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

DrawService::Layer::Layer(Layer&& rhs):
    group(std::move(rhs.group)),
    view(std::move(rhs.view))
{ }

DrawService::Layer& DrawService::Layer::operator= (Layer&& rhs)
{
    group = std::move(rhs.group);
    view  = std::move(rhs.view);
    return *this;
}


DrawService::DrawService(sf::RenderWindow& window, std::size_t layerCount):
    m_window(window), m_layers(layerCount)
{ }

void DrawService::clear()
{
    m_window.clear(m_backgroundColor);
}

void DrawService::draw()
{
    BOOST_FOREACH(Layer& layer, m_layers) {
        m_window.setView(layer.view);
        m_window.draw(layer.group);
    }
}

void DrawService::display()
{
    m_window.display();
}

sf::RenderTarget& DrawService::renderTarget()
{
    return m_window;
}

void DrawService::resetLayerViews()
{
    BOOST_FOREACH(Layer& layer, m_layers)
        layer.view = m_window.getDefaultView();
}

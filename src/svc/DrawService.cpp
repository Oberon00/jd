#include "DrawService.hpp"

#include <SFML/Graphics/RenderWindow.hpp>


DrawService::Layer::Layer(Layer&& rhs):
    view(std::move(rhs.view)),
    group(std::move(rhs.group))
{ }

DrawService::Layer& DrawService::Layer::operator= (Layer&& rhs)
{
    view  = std::move(rhs.view);
    group = std::move(rhs.group);
    return *this;
}


DrawService::DrawService(sf::RenderWindow& window, std::size_t layerCount):
    m_layers(layerCount), m_window(window)
{
    resetLayerViews();
}

void DrawService::clear()
{
    m_window.clear(m_backgroundColor);
}

void DrawService::draw()
{
    for (Layer& layer : m_layers) {
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
    for (Layer& layer : m_layers)
        layer.view = m_window.getDefaultView();
}

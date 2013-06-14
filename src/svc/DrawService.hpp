#ifndef DRAW_SERVICE_HPP_INCLUDED
#define DRAW_SERVICE_HPP_INCLUDED DRAW_SERVICE_HPP_INCLUDED

#include "compsys/Component.hpp"
#include "TransformGroup.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/View.hpp>

#include <vector>


namespace sf { class RenderWindow; class RenderTarget; }

class DrawService: public Component {
    JD_COMPONENT
public:
    struct Layer {
        Layer() { }
        Layer(Layer&& rhs);
        Layer& operator= (Layer&& rhs);

        sf::View view;
        TransformGroup group;

    private:
        Layer& operator= (Layer const&);
    };

    DrawService(sf::RenderWindow& window, std::size_t layerCount = 1);

    sf::RenderTarget& renderTarget();
    Layer& layer(std::size_t n) { return m_layers[n]; }
    std::size_t layerCount() const { return m_layers.size(); }

    void clear();
    void draw();
    void display();

    void resetLayerViews();

    sf::Color backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(sf::Color color) { m_backgroundColor = color; }

private:
    std::vector<Layer> m_layers;
    sf::Color m_backgroundColor;
    sf::RenderWindow& m_window;
};

#endif

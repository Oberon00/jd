#ifndef EVENT_DISPATCHER_HPP_INCLUDED
#define EVENT_DISPATCHER_HPP_INCLUDED EVENT_DISPATCHER_HPP_INCLUDED

#include "compsys/Component.hpp"
#include "ssig.hpp"

#include <SFML/Window/Event.hpp>


namespace sf { class Window; }

class EventDispatcher: public Component {
    JD_COMPONENT

    // Raw
    SSIG_DEFINE_MEMBERSIGNAL(sfEvent, void(sf::Event const&))

    // Misc
    SSIG_DEFINE_MEMBERSIGNAL(closed, void())
    SSIG_DEFINE_MEMBERSIGNAL(resized, void(sf::Event::SizeEvent const&))
    SSIG_DEFINE_MEMBERSIGNAL(lostFocus, void())
    SSIG_DEFINE_MEMBERSIGNAL(gainedFocus, void())
    SSIG_DEFINE_MEMBERSIGNAL(textEntered, void(sf::Event::TextEvent const&))

    // Keyboard
    SSIG_DEFINE_MEMBERSIGNAL(keyPressed, void(sf::Event::KeyEvent const&))
    SSIG_DEFINE_MEMBERSIGNAL(keyReleased, void(sf::Event::KeyEvent const&))

    // Mouse
    SSIG_DEFINE_MEMBERSIGNAL(mouseWheelMoved, void(sf::Event::MouseWheelEvent const&))
    SSIG_DEFINE_MEMBERSIGNAL(mouseButtonPressed, void(sf::Event::MouseButtonEvent const&))
    SSIG_DEFINE_MEMBERSIGNAL(mouseButtonReleased, void(sf::Event::MouseButtonEvent const&))
    SSIG_DEFINE_MEMBERSIGNAL(mouseMoved, void(sf::Event::MouseMoveEvent const&))
    SSIG_DEFINE_MEMBERSIGNAL(mouseEntered, void())
    SSIG_DEFINE_MEMBERSIGNAL(mouseLeft, void())

public:
    EventDispatcher(sf::Window& eventSource);

    void setWaitsForEvents(bool wait = true) { m_waitsForEvents = wait; }
    bool waitsForEvents() const { return m_waitsForEvents; }

    void dispatch();

    bool isWindowFocused() const { return m_hasFocus; }
    bool isKeyPressed(sf::Keyboard::Key k) const;
    sf::Vector2i mousePosition() const;
    bool isMouseButtonPressed(sf::Mouse::Button b) const;

private:
    void dispatchEvent(sf::Event& ev);

    bool m_hasFocus;
    bool m_waitsForEvents;
    sf::Window& m_eventSource;
};

#endif

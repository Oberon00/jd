// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "EventDispatcher.hpp"

#include "Logfile.hpp"

#include <boost/lexical_cast.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>


EventDispatcher::EventDispatcher(sf::Window& eventSource):
    m_hasFocus(true),
    m_waitsForEvents(false),
    m_eventSource(eventSource)
{ }

void EventDispatcher::dispatch()
{
    sf::Event ev;
    if (m_waitsForEvents) {
        if (!m_eventSource.waitEvent(ev)) {
            LOG_W("sf::Window::waitEvent() failed");
            return;
        }
        dispatchEvent(ev);
    }

    while (m_eventSource.pollEvent(ev))
        dispatchEvent(ev);
}

void EventDispatcher::dispatchEvent(sf::Event& ev)
{
    m_sig_sfEvent(ev);
    switch (ev.type) {
        using sf::Event;

        // Misc
        case Event::Closed: m_sig_closed(); break;
        case Event::Resized: m_sig_resized(ev.size); break;
        case Event::LostFocus: m_hasFocus = false; m_sig_lostFocus(); break;
        case Event::GainedFocus: m_hasFocus = true; m_sig_gainedFocus(); break;
        case Event::TextEntered: m_sig_textEntered(ev.text); break;


        // Keyboard
        case Event::KeyPressed: m_sig_keyPressed(ev.key); break;
        case Event::KeyReleased: m_sig_keyReleased(ev.key); break;

        // Mouse
        case Event::MouseWheelMoved: m_sig_mouseWheelMoved(ev.mouseWheel); break;
        case Event::MouseButtonPressed: m_sig_mouseButtonPressed(ev.mouseButton); break;
        case Event::MouseButtonReleased: m_sig_mouseButtonReleased(ev.mouseButton); break;
        case Event::MouseMoved: m_sig_mouseMoved(ev.mouseMove); break;
        case Event::MouseEntered: m_sig_mouseEntered(); break;
        case Event::MouseLeft: m_sig_mouseLeft(); break;

        // Joystick (supported only via the raw sfEvent signal)
        case Event::JoystickConnected:
        case Event::JoystickDisconnected:
        case Event::JoystickButtonPressed:
        case Event::JoystickButtonReleased:
        case Event::JoystickMoved:
        break;

        default:
            LOG_W("unknown sf::Event::type: " + boost::lexical_cast<std::string>(ev.type));
        break;
    }
}

bool EventDispatcher::isKeyPressed(sf::Keyboard::Key k) const
{
    bool const pressed = m_hasFocus && sf::Keyboard::isKeyPressed(k);
    return pressed;
}

sf::Vector2i EventDispatcher::mousePosition() const
{
    return sf::Mouse::getPosition(m_eventSource);
}

bool EventDispatcher::isMouseButtonPressed(sf::Mouse::Button b) const
{
    return m_hasFocus && sf::Mouse::isButtonPressed(b);
}

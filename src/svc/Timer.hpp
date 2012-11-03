#ifndef TIMER_HPP_INCLUDED
#define TIMER_HPP_INCLUDED TIMER_HPP_INCLUDED

#include "compsys/Component.hpp"
#include <string>
#include <boost/function.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>
#include <list>
#include "ssig.hpp"

class Timer: public Component {
    JD_COMPONENT

public:
    typedef boost::function<void()> Callback;
 
private:
    struct Entry {
        Callback callback;
        sf::Time at;
        sf::Time every; // null, if the callback should execute only once
        std::size_t id;
    };

public:
    class CallOrder: public ConnectionBase {
    public:
        virtual void disconnect() override;
        virtual bool isConnected() const override;
    private:
        friend Timer;
        CallOrder(Timer& timer, std::size_t id);

        WeakRef<Timer> m_timer;
        std::size_t m_id;
    };

    Timer();
    
    CallOrder callAfter(sf::Time after, Callback const& callback);
    CallOrder callEvery(sf::Time every, Callback const& callback);

    sf::Time frameDuration() const; // actually the duration of the last frame

    void beginFrame();
    void processCallbacks();
    void endFrame();

    float const factor() const;
    void setFactor(float factor);

private:
    friend CallOrder;
    void cancelOrder(std::size_t id);
    bool hasOrder(std::size_t id);

    std::list<Entry> m_entries;

    sf::Clock m_timer;
    sf::Time m_frameStart;
    sf::Time m_frameTime;
    float m_factor;
    std::size_t m_nextId;
};

#endif


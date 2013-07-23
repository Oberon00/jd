// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "Timer.hpp"

#include "Logfile.hpp"


Timer::Timer():
    m_frameTime(sf::seconds(1/60.f)),
    m_factor(1.f),
    m_nextId(0)
{
}

Timer::CallOrder Timer::callAfter(sf::Time after, Callback const& callback)
{
    std::size_t const id = m_nextId++;
    Entry e = {
        callback,
        m_timer.getElapsedTime() + after, sf::Time::Zero,
        id};
    m_entries.push_back(std::move(e));
    return Timer::CallOrder(*this, id);
}

Timer::CallOrder Timer::callEvery(sf::Time every, Callback const& callback)
{
    if (every <= sf::seconds(1.f / 10))
        throw std::invalid_argument(
        "timer resolution too low. connect to mainloop instead");
    std::size_t const id = m_nextId++;
    Entry e = {
        callback,
        m_timer.getElapsedTime() + every, every,
        id};
    assert(e.at > m_timer.getElapsedTime());
    m_entries.push_back(std::move(e));
    return Timer::CallOrder(*this, id);
}


sf::Time Timer::frameDuration() const
{
    return m_frameTime;
}

void Timer::beginFrame()
{
    m_frameStart = m_timer.getElapsedTime();
}

void Timer::processCallbacks()
{
    auto const time =  m_timer.getElapsedTime() * m_factor;
    for (auto it = m_entries.begin(); it != m_entries.end(); ) {
        if (it->at <= time) {
            if (!it->callback) {
                it = m_entries.erase(it);
            } else {
                it->callback();
                if (it->every > sf::Time::Zero) {
                    it->at += it->every;
                    if (it->at < time) // do not call more than twice in a row
                        it->at = time;
                    ++it;
                } else {
                    it = m_entries.erase(it);
                }
            }
        } else {
            ++it;
        }
    }
}

void Timer::endFrame()
{
    m_frameTime = std::min(
        m_timer.getElapsedTime() - m_frameStart, sf::seconds(1/5.f));
    m_frameTime *= m_factor;
}


float Timer::factor() const
{
    return m_factor;
}

void Timer::setFactor(float factor)
{
    m_factor = factor;
}


void Timer::cancelOrder(std::size_t id)
{
    auto const it = std::find_if(
        m_entries.begin(), m_entries.end(), [id](Entry const& e) {
            return e.id == id;
        });
    if (it == m_entries.end())
        throw std::logic_error("attempt to cancel a timer twice");

    it->callback.clear();
}

bool Timer::hasOrder(std::size_t id)
{
    auto const it = std::find_if(
        m_entries.begin(), m_entries.end(), [id](Entry const& e) {
            return e.id == id;
        });
    return it != m_entries.end();
}

void Timer::CallOrder::disconnect()
{
    if (!m_timer.valid())
        throw std::logic_error("attempt to cancel a CallOrder twice");
    m_timer->cancelOrder(m_id);
    m_timer = static_cast<Component*>(nullptr);
}

bool Timer::CallOrder::isConnected() const
{
    return m_timer.valid() && m_timer->hasOrder(m_id);
}


Timer::CallOrder::CallOrder(Timer& timer, std::size_t id):
    m_timer(timer.ref<Timer>()),
    m_id(id)
{
}

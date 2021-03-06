// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef STATE_MANAGER_HPP_INCLUDED
#define STATE_MANAGER_HPP_INCLUDED STATE_MANAGER_HPP_INCLUDED

#include <boost/function.hpp>

#include <string>
#include <unordered_map>
#include <vector>


class State;

class StateManager {
public:
    StateManager(): m_modifying(false) { }

    //{ Transition functions
    void pushAdditional(State& s);
    void pushAdditional(std::string const& id);

    void push(State& s);
    void push(std::string const& id);

    void pop(); // pop one

    void popRunning(); // pop all running
    void clear();

    // switchTo(s); <=~> pop(); push(s);
    void switchTo(State& s);
    void switchTo(std::string const& id);
    //}

    State* active();

    std::vector<State*> const running() const;
    std::vector<State*> const& stacked() const;

    void registerState(std::string const& id, State& s);
    State* stateForId(std::string const& id);

    typedef boost::function<State*(std::string const&)> StateNotFoundCallback;
    StateNotFoundCallback setStateNotFoundCallback(StateNotFoundCallback const&);

private:
    bool m_modifying;
    std::vector<State*> m_stack;
    std::vector<unsigned> m_concurrentCount;
    std::unordered_map<std::string, State*> m_registered;
    StateNotFoundCallback m_callback;
};

#endif

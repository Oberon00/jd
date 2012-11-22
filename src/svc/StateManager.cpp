#include "StateManager.hpp"
#include "State.hpp"

namespace {

struct Modify {
    Modify(bool& b, std::string const& fn):
        m_b(b)
    {
        if (b)
            throw std::logic_error(fn +
                ": cannot enter two modifying StateManager methods concurrently");
        m_b = true;
    }

    ~Modify() { m_b = false; }

private:
    Modify& operator= (Modify const&);

    bool& m_b;
};
} // anonymous namespace

//{ Transition functions

#define STRINGID_WRAPPER(fname) \
    void StateManager::fname(std::string const& id) \
    {                                                        \
        State* s = stateForId(id);                           \
        if (s)                                               \
            fname(*s);                              \
        else                                                 \
            throw std::runtime_error(__FUNCTION__ ": invalid state id"); \
    }

STRINGID_WRAPPER(push)
STRINGID_WRAPPER(pushAdditional)
STRINGID_WRAPPER(switchTo)

void StateManager::pushAdditional(State& s)
{
#define MODIFYING Modify(m_modifying, __FUNCTION__);
    s.initialize();

    MODIFYING
    if (m_stack.empty())
        push(s);
    m_stack.back()->pause();
    s.prepare();
    m_stack.push_back(&s);
    ++m_concurrentCount.back();
    s.resume();
}

void StateManager::push(State& s)
{
    s.initialize();

    MODIFYING
    if (!m_stack.empty()) {
        m_stack.back()->pause();
        m_stack.back()->stop();
    }
    s.prepare();
    m_stack.push_back(&s);
    m_concurrentCount.push_back(1);
    s.resume();
}

        
void StateManager::pop()
{
    if (m_stack.empty())
        throw std::logic_error(__FUNCTION__ ": stack already empty.");
    if (m_concurrentCount.back() - 1 == 0) {
        popRunning();
    } else {
        MODIFYING

        m_stack.back()->pause();
        m_stack.back()->stop();
        m_stack.back()->cleanup();
        --m_concurrentCount.back();
        m_stack.pop_back();
        m_stack.back()->resume();
    }
}

void StateManager::popRunning()
{
    MODIFYING
#undef MODIFYING

    if (m_stack.empty())
        throw std::logic_error(__FUNCTION__ ": stack already empty.");

    auto const firstPopped = m_stack.end() - m_concurrentCount.back();
    for (auto it = firstPopped; it != m_stack.end(); ++it) {
        (*it)->pause();
        (*it)->stop();
        (*it)->cleanup();
    }
    // prepare previous active
    if (m_stack.size() - m_concurrentCount.back() >= 1) {
        for (auto it = firstPopped - m_concurrentCount[m_concurrentCount.size() - 2];
             it < firstPopped; ++it)
            (*it)->prepare();
    }
    m_stack.erase(m_stack.end() - m_concurrentCount.back(), m_stack.end());
    m_concurrentCount.pop_back();
    if (!m_stack.empty())
        m_stack.back()->resume();
}

void StateManager::switchTo(State& s)
{
    if (!m_stack.empty())
        popRunning();
    push(s);
}
 
//}
        
State* StateManager::active()
{
    return m_stack.empty() ? nullptr : m_stack.back();
}
        
std::vector<State*> const StateManager::running() const
{
    assert(m_concurrentCount.size() >= m_stack.size());
    return m_stack.empty() ? std::vector<State*>() : std::vector<State*>(
        m_stack.end() - m_concurrentCount.back(), m_stack.end());
}

std::vector<State*> const& StateManager::stacked() const
{
    return m_stack;
}

void StateManager::registerState(std::string const& id, State& s)
{
    m_registered.insert(std::make_pair(id, &s));
}

State* StateManager::stateForId(std::string const& id)
{
    auto const it = m_registered.find(id);
    if (it != m_registered.end())
        return it->second;
    if (m_callback) {
        State* s = m_callback(id);
        if (s) {
            registerState(id, *s);
            return s;
        }
    }
    return nullptr;
}

void StateManager::clear()
{
    while (!m_stack.empty())
        popRunning();
}

StateManager::StateNotFoundCallback StateManager::setStateNotFoundCallback(
    StateNotFoundCallback const& callback)
{
    StateNotFoundCallback old(std::move(m_callback));
    m_callback = callback;
    return old;
}

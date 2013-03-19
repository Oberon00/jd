#ifndef SERVICE_LOCATOR_HPP_INCLUDED
#define SERVICE_LOCATOR_HPP_INCLUDED SERVICE_LOCATOR_HPP_INCLUDED

#include <stdexcept>
#include <unordered_map>


class Configuration;
class StateManager;
class LuaVm;
class Mainloop;
class DrawService;
class EventDispatcher;
class Timer;
class SoundManager;

class Component;
class MetaComponent;

typedef Component Service;

class ServiceLocator {
public:
    class Error: public std::runtime_error {
    public:
        Error(std::string const& msg): std::runtime_error(msg) { }
    };

    template<typename T>
    static T& get()
    {
        return ServiceLocator::get(T::staticMetaComponent).template as<T>();
    }

    static Service& get (MetaComponent const& m);

    static void registerService(Service& s);

    static StateManager& stateManager();
    static LuaVm& luaVm();
    static Mainloop& mainloop();
    static DrawService& drawService();
    static EventDispatcher& eventDispatcher();
    static Timer& timer();
    static SoundManager& soundManager();
    static Configuration& configuration();

private:
    static std::unordered_map<MetaComponent const*, Service*> registry;
};

#endif

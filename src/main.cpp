#include "cmdline.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include "State.hpp"
#include "svc/LuaVm.hpp"
#include "svc/FileSystem.hpp"
#include "svc/ServiceLocator.hpp"
#include "svc/Mainloop.hpp"
#include "svc/DrawService.hpp"
#include "svc/EventDispatcher.hpp"
#include "svc/Configuration.hpp"
#include "svc/StateManager.hpp"
#include "svc/Timer.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <luabind/function.hpp> // call_function (used @ loadStateFromLua)
#include <boost/bind.hpp>
#include "Logfile.hpp"
#include <physfs.h>
#include "base64.hpp"
#include "LuaUtils.hpp"
#include <array>
#include <luabind/adopt_policy.hpp>

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <Windows.h>
#   endif

#if defined(BOOST_MSVC) && defined(_DEBUG) && defined(JD_HAS_VLD)
#   include <vld.h>
#endif

namespace {


static std::vector<std::string> cmdLine;
static int argc_ = 0;
static const char* const* argv_ = nullptr;

template<typename T>
struct ServiceEntry: public T
{
    ServiceEntry()
    {
        ServiceLocator::registerService(*this);
    }
};

static State* loadStateFromLua(std::string const& name)
{
    std::string const filename = "lua/states/" + name + ".lua";
    if (!PHYSFS_exists(filename.c_str()))
        return nullptr;

    LOG_D("Loading state \"" + name + "\"...");
    State* s = nullptr;
    try {
        lua_State* L = ServiceLocator::luaVm().L();
        luaU::load(ServiceLocator::luaVm().L(), filename);
        luabind::object chunk(luabind::from_stack(L, -1));
        lua_pop(L, 1);

        // The state must stay alive as long as the StateManager.
        // However, if we simply use luabind::adopt here, we cause a memory
        // leak. So, instead we bind the lifetime of the state to the one
        // from the lua_State (== LuaVm in this case). As long as the LuaVm is
        // destroyed (shortly) *after* the StateManager it just works.
        // Otherwise, we would have a real problem: how could you possibly keep
        // a Lua object alive, longer than it's lua_State?
        luabind::object sobj = chunk();
        s = luabind::object_cast<State*>(sobj);
        sobj.push(L);
        luaL_ref(L, LUA_REGISTRYINDEX);
    } catch (luabind::cast_failed const& e) {
        LOG_E("failed casting lua value to " + std::string(e.info().name()));
    } catch (luabind::error const& e) {
        LOG_E(luaU::Error(e, "failed loading state \"" + name + '\"').what());
    } catch (std::exception const& e) {
        LOG_EX(e);
    }
    log().write(
        "Loading State " + name + (s ? " finished." : " failed."),
        s ? loglevel::debug : loglevel::error, LOGFILE_LOCATION);
    return s;
}

} // anonymous namespace

int argc() { return argc_; }
const char* const* argv() { return argv_; }
std::vector<std::string> const& commandLine() { return cmdLine; }

int main(int argc, char* argv[])
{
    // First thing to do: get the logfile opened.

    // Create the directory for the logfile.
#   ifdef _WIN32
    boost::filesystem::path const logpath(std::string(getenv("APPDATA")) + "/JadeEngine/log.html");
#   else
    boost::filesystem::path const logpath("~/.jade/log.html");
#endif
    boost::filesystem::create_directories(logpath.parent_path());

    int r = EXIT_FAILURE;
    try {

        // Open the logfile
        log().setMinLevel(loglevel::debug);
        log().open(logpath.string());

#ifndef NDEBUG
        LOG_I("This is a debug build.");
#endif
        LOG_D("Initialization...");

        // setup commandline functions //
        argc_ = argc;
        argv_ = argv;
        cmdLine.assign(argv, argv + argc);

        // Construct and register services // 
        auto const regSvc = ServiceLocator::registerService;

        LOG_D("Initializing virtual filesystem...");
        FileSystem::Init fsinit;
        regSvc(FileSystem::get());
        LOG_D("Finished initializing virtual filesystem.");
        
        LOG_D("Initializing Lua...");
        ServiceEntry<LuaVm> luaVm;
        try {
            luaVm.initLibs();
            LOG_D("Finished initializing Lua.");

            ServiceEntry<Mainloop> mainloop;
            ServiceEntry<Configuration> conf;
            ServiceEntry<Timer> timer;

            // Create the RenderWindow now, because some services depend on it.
            LOG_D("Preparing window and SFML...");
            sf::RenderWindow window;
            LOG_D("Finished preparing window and SFML.");

            ServiceEntry<StateManager> stateManager;
            EventDispatcher eventDispatcher(window);
            regSvc(eventDispatcher);

            using boost::bind;
            mainloop.connect_processInput(
                bind(&EventDispatcher::dispatch, &eventDispatcher));

            // Various other initializations //
            ServiceLocator::stateManager().setStateNotFoundCallback(&loadStateFromLua);

            LOG_D("Loading configuration...");
            conf.load();
            LOG_D("Finished loading configuration.");

            DrawService drawService(window, conf.get<std::size_t>("misc.layerCount", 1UL));
            regSvc(drawService);
            mainloop.connect_preFrame(bind(&Timer::beginFrame, &timer));
            mainloop.connect_update(bind(&Timer::processCallbacks, &timer));
            mainloop.connect_preDraw(bind(&DrawService::clear, &drawService));
            mainloop.connect_draw(bind(&DrawService::draw, &drawService));
            mainloop.connect_postDraw(bind(&DrawService::display, &drawService));
            mainloop.connect_postFrame(bind(&Timer::endFrame, &timer));


            LOG_D("Creating window...");
            std::string const title =
                conf.get<std::string>("misc.title", "Jade");
            window.create(
                conf.get("video.mode", sf::VideoMode(800, 600)),
                title,
                conf.get("video.fullscreen", false) ?
                   sf::Style::Close | sf::Style::Fullscreen :  sf::Style::Default);
            window.setVerticalSyncEnabled(conf.get("video.vsync", true));
            window.setFramerateLimit(conf.get("video.framelimit", 0U));
            LOG_D("Finished creating window.");

            drawService.resetLayerViews();

            timer.callEvery(sf::milliseconds(600), [&timer, &window, &title]() {
                std::string const fps = boost::lexical_cast<std::string>(
                    1.f / timer.frameDuration().asSeconds());
                window.setTitle(title + " [" + fps + " fps]");
            });

            timer.callEvery(sf::seconds(60), [&timer]() {
                std::string const fps = boost::lexical_cast<std::string>(
                    1.f / timer.frameDuration().asSeconds());
                LOG_D("Current framerate (fps): " + fps);
            });

            // Execute init.lua //
#           define INIT_LUA "lua/init.lua"
            LOG_D("Executing \"" INIT_LUA "\"...");
            luaU::exec(luaVm.L(), INIT_LUA);
            LOG_D("Finished executing \"" INIT_LUA "\".");
#           undef INIT_LUA

            LOG_D("Finished initialization.");

            // Run mainloop //
            LOG_D("Mainloop...");
            r = ServiceLocator::mainloop().exec();
            LOG_D("Mainloop finished with exit code " + boost::lexical_cast<std::string>(r) + ".");

            LOG_D("Cleanup...");
            stateManager.clear();
			luaVm.deinit();

        } catch (luabind::error const& e) {
            throw luaU::Error(e);
        } catch (luabind::cast_failed const& e) {
            throw luaU::Error(e.what() + std::string("; type: ") + e.info().name());
        }

    // In case of an exception, log it and notify the user //
    } catch (std::exception const& e) {
        log().logEx(e, loglevel::fatal, LOGFILE_LOCATION);
#       ifdef _WIN32
        MessageBoxA(NULL, e.what(), "Jade Engine", MB_ICONERROR | MB_TASKMODAL);
#       else
        std::cerr << "Exception: " << e.what();
#       endif
        return EXIT_FAILURE;
    }

    LOG_D("Cleanup finished.");
    return r;
}

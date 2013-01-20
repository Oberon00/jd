#include "cmdline.hpp"

#include "Logfile.hpp"
#include "LuaUtils.hpp"
#include "ResourceLoaders.hpp"
#include "ResourceManager.hpp"
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
#include "svc/SoundManager.hpp"

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/function.hpp> // call_function (used @ loadStateFromLua)
#include <physfs.h>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#if defined(BOOST_MSVC) && defined(_DEBUG) && defined(JD_HAS_VLD)
#   include <vld.h>
#endif

#ifdef _WIN32
#   include <fcntl.h>
#   include <io.h>
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <Windows.h>
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

#ifdef _WIN32
static int openOfsStdHandle(DWORD nStdHandle, int flags = _O_TEXT)
{
    HANDLE const handle = GetStdHandle(nStdHandle);
    if (!handle || handle  == INVALID_HANDLE_VALUE)
        return -1;
    return _open_osfhandle(
        reinterpret_cast<intptr_t>(handle),
        flags);
}

static void redirectHandle(DWORD nStdHandle, FILE* stdfile, char const* name)
{
    int const handleFileNo = openOfsStdHandle(nStdHandle);
    if (handleFileNo == -1)
        LOG_W("Opening " + std::string(name) + " from Win32 STD_HANDLE failed.");
    else if (_dup2(handleFileNo, _fileno(stdfile)) != 0)
        LOG_W("Redirecting " + std::string(name) + " to Win32 STD_HANDLE failed.");
}

static bool attachToConsole()
{
    // ERROR_ACCESS_DENIED: Already owns console.
    return AttachConsole(ATTACH_PARENT_PROCESS) != 0 || GetLastError() == ERROR_ACCESS_DENIED;
}

static void initializeStdStreams(std::string const& basepath)
{
    if (attachToConsole()) {
        LOG_D("Attaching to console.");
        redirectHandle(STD_OUTPUT_HANDLE, stdout, "stdout");
        redirectHandle(STD_ERROR_HANDLE, stderr, "stderr");
        redirectHandle(STD_INPUT_HANDLE, stdin, "stdin");
    } else {
        if (!std::freopen((basepath + "/stdout.txt").c_str(), "w", stdout))
            LOG_W("Redirecting stdout to file failed.");
        if (!std::freopen((basepath + "/stderr.txt").c_str(), "w", stderr))
            LOG_W("Redirecting stderr to file failed.");
    }
}
#else
inline void initializeStdStreams(std::string const&)
{
    // Not needed: do nothing.
}
#endif

} // anonymous namespace

int argc() { return argc_; }
const char* const* argv() { return argv_; }
std::vector<std::string> const& commandLine() { return cmdLine; }

int main(int argc, char* argv[])
{
    assert(argc > 0);
    std::string const defaultGame = argv[0]; // Adjust if neccessary.
    std::string game = defaultGame;
    bool gameSpecified = false;
    if (argc >= 2) {
        game = argv[1];
        gameSpecified = true;
    }
    const boost::filesystem::path gamePath(game);
    std::string gameName = (gamePath.has_stem() ?
        gamePath.stem() : gamePath.has_filename() ?
            gamePath.filename() : gamePath.parent_path().filename()).string();

    bool gameNameFound = true;
    if (gameName.empty() || gameName == ".") {
        gameName = "JadeEngine";
        gameNameFound = false;
    }
    
    // First thing to do: get the logfile opened.

    // Create directory for log file
#   ifdef _WIN32
    std::string const basepath(std::string(getenv("APPDATA")) + '/' + gameName + '/');
#   else
    std::string const basepath("~/." + gameName + "/");
#endif

    int r = EXIT_FAILURE;
    try {
        boost::filesystem::create_directories(basepath);

        // Open the logfile
        log().setMinLevel(loglevel::debug);
        log().open(basepath + "jd.log");

#ifndef NDEBUG
        LOG_I("This is a debug build.");
#endif
        LOG_I("Game name is \"" + gameName + "\".");
        if (!gameNameFound)
            LOG_W("This is the fallback name: no proper one was found.");

        LOG_D("Initialization...");
        initializeStdStreams(basepath);

        // setup commandline functions //
        argc_ = argc;
        argv_ = argv;
        cmdLine.assign(argv, argv + argc);

        // Construct and register services // 
        auto const regSvc = ServiceLocator::registerService;

        LOG_D("Initializing virtual filesystem...");
        FileSystem::Init fsinit;

        FileSystem& fs = FileSystem::get();
        regSvc(fs);

        std::string programpath = PHYSFS_getBaseDir();
        fs.mount(programpath + "/data", "/",
            FileSystem::mountOptional);
        std::string const baselib = programpath + "/base.jd";
        if (!fs.mount(baselib, "/", FileSystem::mountOptional)) {
            FileSystem::Error const err("Could not open \"" + baselib + '\"');
            if (!fs.mount("./base.jd", "/", FileSystem::mountOptional)) {
                LOG_W(err.what());
                LOG_W(FileSystem::Error(
                    "Could not open \"./base.jd\"").what());
            }
        }

        if (!fs.mount(game, "/", gameSpecified ?
            FileSystem::prependPath : FileSystem::mountOptional)
        ) {
            LOG_W(FileSystem::Error(
                "Failed mounting game \"" + game + "\"").what());
            LOG_W("Mounting working directory instead.");
            fs.mount(".");
        }

        fs.mount(basepath + "data/", "/",
            FileSystem::writeDirectory|FileSystem::prependPath);


        LOG_D("Finished initializing virtual filesystem.");

        initDefaultResourceLoaders();

        LOG_D("Initializing Lua...");
        ServiceEntry<LuaVm> luaVm;
        try {
            luaVm.initLibs();
            LOG_D("Finished initializing Lua.");

            ServiceEntry<Mainloop> mainloop;
            ServiceEntry<Configuration> conf;
            ServiceEntry<Timer> timer;
            ServiceEntry<SoundManager> sound;
            ServiceEntry<StateManager> stateManager;

            LOG_D("Loading configuration...");
            conf.load();
            LOG_D("Finished loading configuration.");


            // Create the RenderWindow now, because some services depend on it.
            LOG_D("Creating Window and preparing SFML...");
            std::unique_ptr<sf::RenderWindow> window;
            luaU::load(luaVm.L(), "initwindow.lua");
            { // Scope for chunk.
                luabind::object chunk(luabind::from_stack(luaVm.L(), -1));
                lua_pop(luaVm.L(), 1);
                window.reset(luabind::call_function<sf::RenderWindow*>(chunk)
                    [luabind::adopt(luabind::result)]);
            }
            if (!window)
                throw std::runtime_error("Got a NULL-window.");
            LOG_D("Finished creating Window and preparing SFML.");

            EventDispatcher eventDispatcher(*window);
            regSvc(eventDispatcher);

            using boost::bind;
            mainloop.connect_processInput(
                bind(&EventDispatcher::dispatch, &eventDispatcher));

            // Various other initializations //
            ServiceLocator::stateManager().setStateNotFoundCallback(&loadStateFromLua);

            DrawService drawService(*window, conf.get<std::size_t>("misc.layerCount", 1UL));
            regSvc(drawService);
            mainloop.connect_preFrame(bind(&Timer::beginFrame, &timer));
            mainloop.connect_update(bind(&Timer::processCallbacks, &timer));
            mainloop.connect_update(bind(&SoundManager::fade, &sound));
            mainloop.connect_preDraw(bind(&DrawService::clear, &drawService));
            mainloop.connect_draw(bind(&DrawService::draw, &drawService));
            mainloop.connect_postDraw(bind(&DrawService::display, &drawService));
            mainloop.connect_postFrame(bind(&Timer::endFrame, &timer));

            timer.callEvery(sf::seconds(10), bind(&SoundManager::tidy, &sound));

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

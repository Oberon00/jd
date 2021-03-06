// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "cmdline.hpp"

#include "Logfile.hpp"
#include "luaUtils.hpp"
#include "ressys/resourceLoaders.hpp"
#include "ressys/ResourceManager.hpp"
#include "State.hpp"
#include "svc/LuaVm.hpp"
#include "svc/FileSystem.hpp"
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
#include <boost/locale/generator.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/function.hpp> // call_function (used @ loadStateFromLua)
#include <luabind/back_reference.hpp>
#include <physfs.h>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <iostream> // for cout, clog, cerr, cin .imbue()
#include <locale>

#if defined(BOOST_MSVC) && defined(_DEBUG) && defined(JD_HAS_VLD)
#   include <vld.h>
#endif

#ifdef _WIN32
#   include <fcntl.h>
#   include <io.h>
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <Windows.h>
#   include <Shellapi.h> // CommandLineToArgvW
#endif

namespace fs = boost::filesystem;
namespace conv = boost::locale::conv;

namespace {


static std::vector<std::string> cmdLine;
static int argc_ = 0;
static const char* const* argv_ = nullptr;

class LoadStateFromLua {
public:
    LoadStateFromLua(lua_State* L): m_L(L) {}
    
    State* operator() (std::string const& name)
    {
        std::string const filename = "lua/states/" + name + ".lua";
        if (!PHYSFS_exists(filename.c_str()))
            return nullptr;

        LOG_D("Loading state \"" + name + "\"...");
        State* s = nullptr;
        try {
            luaU::load(m_L, filename);
            luabind::object chunk(luabind::from_stack(m_L, -1));
            lua_pop(m_L, 1);

            // The state must stay alive as long as the StateManager.
            // However, if we simply use luabind::adopt here, we cause a memory
            // leak. So, instead we bind the lifetime of the state to the one
            // from the lua_State (== LuaVm in this case). As long as the LuaVm is
            // destroyed (shortly) *after* the StateManager it just works.
            // Otherwise, we would have a real problem: how could you possibly keep
            // a Lua object alive, longer than it's lua_State?
            luabind::object sobj = chunk();
            s = luabind::object_cast<State*>(sobj);
            sobj.push(m_L);
            luaL_ref(m_L, LUA_REGISTRYINDEX);
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
private:
    lua_State* m_L;
};

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
    return AttachConsole(ATTACH_PARENT_PROCESS) != 0 ||
           GetLastError() == ERROR_ACCESS_DENIED;
}



static void initializeStdStreams(std::string const& basepath)
{
    std::wstring wbasepath = conv::utf_to_utf<wchar_t>(basepath);
    if (attachToConsole()) {
        LOG_D("Attaching to console.");
        redirectHandle(STD_OUTPUT_HANDLE, stdout, "stdout");
        redirectHandle(STD_ERROR_HANDLE, stderr, "stderr");
        redirectHandle(STD_INPUT_HANDLE, stdin, "stdin");
    } else {
        if (!_wfreopen((wbasepath + L"/stdout.txt").c_str(), L"w", stdout))
            LOG_W("Redirecting stdout to file failed.");
        if (!_wfreopen((wbasepath + L"/stderr.txt").c_str(), L"w", stderr))
            LOG_W("Redirecting stderr to file failed.");
    }
}

static void reportError(std::string const& msg)
{
    std::wstring const wmsg = conv::utf_to_utf<wchar_t>(msg);
    MessageBoxW(
        NULL,                           // hWnd
        wmsg.c_str(),                   // lpText
        L"Jade Engine: Error",          // lpCaption
        MB_ICONERROR | MB_TASKMODAL);   // uType
}

#else
inline void initializeStdStreams(std::string const&)
{
    // Not needed: do nothing.
}

static void reportError(std::string const& msg)
{
    std::cerr << msg << std::endl;
}
#endif

} // anonymous namespace

int argc() { return argc_; }
const char* const* argv() { return argv_; }
std::vector<std::string> const& commandLine() { return cmdLine; }

int main(int argc, char* argv[])
{
    int r = EXIT_FAILURE;
    std::string logpath; // Used in error reporting.
    try {
        assert(argc > 0);

        // setup commandline functions //
        argc_ = argc;
        argv_ = argv;

#ifdef _WIN32
        wchar_t* wcmdLine = GetCommandLineW();
        wchar_t** wargv = CommandLineToArgvW(wcmdLine, &argc);
        cmdLine.reserve(argc);
        for (int i = 0; i < argc; ++i) {
            cmdLine.push_back(conv::utf_to_utf<char>(wargv[i]));
        }
#else
        cmdLine.assign(argv, argv + argc);
#endif

        std::locale::global(boost::locale::generator().generate("")); // Use UTF-8
        fs::path::imbue(std::locale()); // Not sure if this is necessary.
        std::cout.imbue(std::locale());
        std::cerr.imbue(std::locale());
        std::clog.imbue(std::locale());
        std::cin.imbue(std::locale());

#ifdef _WIN32
        std::string defaultGame;
        { // Scope for moduleName, moduleNameW
            uint16_t moduleNameW[MAX_PATH];
            if (GetModuleFileNameW(
                    NULL,
                    reinterpret_cast<wchar_t*>(moduleNameW),
                    MAX_PATH) != 0) {
                defaultGame = conv::utf_to_utf<char>(moduleNameW);
            } else {
                defaultGame = commandLine()[0];
            }
        }
#else
        std::string const defaultGame = argv[0]; // Adjust if necessary.
#endif
        std::string game = defaultGame;
        bool gameSpecified = false;
        if (argc >= 2) {
            game = commandLine()[1];
            gameSpecified = true;
        }
        fs::path gamePath = game;
        gamePath.make_preferred(); // This path may appear in error messages.
        std::string gameName = (
            gamePath.has_stem() ?
                gamePath.stem() : gamePath.has_filename() ?
                gamePath.filename() : gamePath.parent_path().filename()
        ).string();

        bool gameNameFound = true;
        if (gameName.empty() || gameName == ".") {
            gameName = "JadeEngine";
            gameNameFound = false;
        }


        // Create directory for log file
#ifdef _WIN32
        std::string const basepath = fs::path(
            conv::utf_to_utf<char>(_wgetenv(L"APPDATA")) + '/' + gameName + '/'
        ).make_preferred().string();

#else
        std::string const basepath(std::string(getenv("HOME")) + "/." + gameName +  '/');
#endif

        logpath = basepath + "jd.log";

        fs::create_directories(basepath);

        // Open the logfile
        log().setMinLevel(loglevel::debug);
        log().open(logpath);

        LOG_I(
            "Jade Engine -- Copyright (c) Christian Neumüller 2012--2013\n"
            "This program is subject to the terms of the BSD 2-Clause License.\n"
            "See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause");
#ifndef NDEBUG
        LOG_I("This is a debug build.");
#endif
        LOG_I("Game name is \"" + gameName + "\".");
        if (!gameNameFound)
            LOG_W("This is the fallback name: no proper one was found.");

        LOG_D("Initialization...");
        initializeStdStreams(basepath);

        // Construct and register services //

        LOG_D("Initializing virtual filesystem...");
        vfs::Init fsinit;

        fs::path programpath = PHYSFS_getBaseDir();
        std::vector<std::string> baselibpaths;
        baselibpaths.push_back((programpath / "base.jd").string());
        baselibpaths.push_back((programpath / "../base.jd/base.jd").string());
        baselibpaths.push_back((programpath / "../base.jd").string());
        baselibpaths.push_back((programpath / "../share/jade-engine/base.jd").string());
        baselibpaths.push_back((programpath / "../share/base.jd").string());
        baselibpaths.push_back(basepath + "/base.jd");
        baselibpaths.push_back("./base.jd");
        vfs::mountFirstWorking(baselibpaths, "/", vfs::logWarnings|vfs::mountOptional);

        if (!vfs::mount(game, "/", vfs::logWarnings|(gameSpecified ?
            vfs::prependPath : vfs::mountOptional))
        ) {
            LOG_W("Mounting working directory instead.");
            vfs::mount(".");
        }

        vfs::mount(basepath + "data/", "/",
            vfs::writeDirectory|vfs::prependPath);


        LOG_D("Finished initializing virtual filesystem.");

        initDefaultResourceLoaders();

        LOG_D("Initializing Lua...");
        LuaVm luaVm;
        try {
            luaVm.initLibs();
            LOG_D("Finished initializing Lua.");
            
            luabind::object svctable = luabind::newtable(luaVm.L());
            luabind::object jdtable = luabind::rawget(
                luabind::globals(luaVm.L()), "jd");
            luabind::rawset(jdtable, "svc", svctable);
           
            Mainloop mainloop;
            luabind::rawset(svctable, "mainloop", &mainloop);
            
            StateManager stateManager;
            luabind::rawset(svctable, "stateManager", &stateManager);
            
            Configuration conf(luaVm.L());
            luabind::rawset(svctable, "configuration", &conf);
            
            SoundManager sound;
            luabind::rawset(svctable, "soundManager", &sound);
            
            Timer timer;
            luabind::rawset(svctable, "timer", &timer);

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
            luabind::rawset(svctable, "eventDispatcher", &eventDispatcher);

            using boost::bind;
            mainloop.connect_processInput(
                bind(&EventDispatcher::dispatch, &eventDispatcher));

            // Various other initializations //
            stateManager.setStateNotFoundCallback(LoadStateFromLua(luaVm.L()));

            DrawService drawService(
                *window, conf.get<std::size_t>("misc.layerCount", 1UL));
            luabind::rawset(svctable, "drawService", &drawService);
            
            mainloop.connect_preFrame(bind(&Timer::beginFrame, &timer));
            mainloop.connect_update(bind(&Timer::processCallbacks, &timer));
            mainloop.connect_update([&timer, &sound]() {
                sound.fade(timer.frameDuration());
            });
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
            r = mainloop.exec();
            LOG_D("Mainloop finished with exit code " +
                  boost::lexical_cast<std::string>(r) + ".");

            LOG_D("Cleanup...");
            stateManager.clear();
            luaVm.deinit();

        } catch (luabind::error const& e) {
            throw luaU::Error(e);
        } catch (luabind::cast_failed const& e) {
            throw luaU::Error(
                e.what() + std::string("; type: ") + e.info().name());
        }

    // In case of an exception, log it and notify the user //
    } catch (std::exception const& e) {
        std::string msg = "An exception occured: " + std::string(e.what());
        if (logpath.empty()) {
            msg += "\n\nAlso, the logfile could not be created/updated.";
        } else {
            try {
                log().logEx(e, loglevel::fatal, LOGFILE_LOCATION);
            } catch (std::exception const& e2) {
                reportError(
                    e2.what() + std::string(
                        " (occured when trying to log the following error)"));
            }
            msg +=  "\n\nThe logfile at ";
            msg += logpath;
            msg += " may contain more information.";
        }
        reportError(msg);
        return EXIT_FAILURE;
    }

    LOG_D("Cleanup finished.");
    return r;
}

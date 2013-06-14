set(SVC_HEADERS
    svc/Mainloop.hpp
    svc/StateManager.hpp
    svc/LuaVm.hpp
    svc/FileSystem.hpp
    svc/EventDispatcher.hpp
    svc/DrawService.hpp
    svc/Configuration.hpp
    svc/ServiceLocator.hpp
    svc/Timer.hpp
    svc/SoundManager.hpp)
source_group("Service Headers" FILES ${SVC_HEADERS})

set(SVC_SOURCES
    svc/Mainloop.cpp
    svc/StateManager.cpp
    svc/LuaVm.cpp
    svc/FileSystem.cpp
    svc/EventDispatcher.cpp
    svc/DrawService.cpp
    svc/Configuration.cpp
    svc/ServiceLocator.cpp
    svc/Timer.cpp
    svc/SoundManager.cpp)
source_group("Service Sources" FILES ${SVC_SOURCES})

set(COMPSYS_HEADERS
    compsys/Component.hpp
    compsys/MetaComponent.hpp
    compsys/BasicMetaComponent.hpp
    compsys/ComponentRegistry.hpp
    compsys/Entity.hpp)
source_group("Component System Headers" FILES ${COMPSYS_HEADERS})

set(COMPSYS_SOURCES
    compsys/MetaComponent.cpp
    compsys/Component.cpp
    compsys/ComponentRegistry.cpp
    compsys/Entity.cpp)
source_group("Component System Sources" FILES ${COMPSYS_SOURCES})

set(COMP_HEADERS
    comp/PositionComponent.hpp
    comp/TileCollisionComponent.hpp
    comp/TilePositionComponent.hpp
    comp/RectCollisionComponent.hpp)
source_group("Component Headers" FILES ${COMP_HEADERS})

set(COMP_SOURCES
    comp/PositionComponent.cpp
    comp/TileCollisionComponent.cpp
    comp/TilePositionComponent.cpp)
source_group("Component Sources" FILES ${COMP_SOURCES})

set(LUAEXPORT_HEADERS
    luaexport/ExportThis.hpp
    luaexport/LuaExportMacros.hpp
    luaexport/LuaFunction.hpp
    luaexport/LuaEventHelpers.hpp
    luaexport/SfBaseTypes.hpp
    luaexport/sharedPtrConverter.hpp
    luaexport/container.hpp
    luaexport/resources.hpp)
source_group("LuaExport Headers" FILES ${LUAEXPORT_HEADERS})

set(LUAEXPORT_SOURCES
    luaexport/LuaExtra.cpp
    luaexport/SfSystem.cpp
    luaexport/SfWindow.cpp
    luaexport/SfGraphics.cpp
    luaexport/Logfile.cpp
    luaexport/EntitySystem.cpp
    luaexport/ServiceLocator.cpp
    luaexport/Collisions.cpp
    luaexport/Geometry.cpp
    luaexport/FileSystemMeta.cpp
    luaexport/DrawServiceMeta.cpp
    luaexport/MainloopMeta.cpp
    luaexport/StateManagerMeta.cpp
    luaexport/State.cpp
    luaexport/TileCollisionComponentMeta.cpp
    luaexport/EventDispatcherMeta.cpp
    luaexport/TimerMeta.cpp
    luaexport/LuaPackage.cpp
    luaexport/Tilemap.cpp
    luaexport/TilePositionComponentMeta.cpp
    luaexport/RectCollisionComponentMeta.cpp
    luaexport/luaIo.cpp
    luaexport/SoundManagerMeta.cpp)
source_group("LuaExport Sources" FILES ${LUAEXPORT_SOURCES})


set(JD_HEADERS
    encoding.hpp
    ssig.hpp
    ssig_template.hpp
    MapInfo.hpp
    exceptions.hpp
    resourceLoaders.hpp
    resfwd.hpp
    AutoResource.hpp
    AutoTexture.hpp
    AutoFont.hpp
    AutoSoundBuffer.hpp
    ResourceManager.hpp
    ResourceManager.inl
    sfKeyCodes.hpp
    jdConfig.hpp
    cmdline.hpp
    WeakRef.hpp
    Tilemap.hpp
    TransformGroup.hpp
    Logfile.hpp
    sfUtil.hpp
    Collisions.hpp
    TileCollideableGroup.hpp
    RectCollideableGroup.hpp
    State.hpp
    base64.hpp
    luaUtils.hpp
    VFileFont.hpp
    VFileMusic.hpp
    ${SVC_HEADERS}
    ${COMPSYS_HEADERS}
    ${COMP_HEADERS}
    ${LUAEXPORT_HEADERS})

set(JD_SOURCES
    encoding.cpp
    MapInfo.cpp
    main.cpp
    resourceLoaders.cpp
    luaUtils.cpp
    jdConfig.cpp
    Tilemap.cpp
    TransformGroup.cpp
    Logfile.cpp
    Collisions.cpp
    TileCollideableGroup.cpp
    RectCollideableGroup.cpp
    base64.cpp
    sfUtil.cpp
    Resources.rc
    ${SVC_SOURCES}
    ${COMPSYS_SOURCES}
    ${COMP_SOURCES}
    ${LUAEXPORT_SOURCES})

set(COMP_DEFS "SFML_STATIC")

if (MSVC)
    add_definitions(
        /wd4251 # disable warnings about undefined DLL interfaces (luabind)
        /wd4709 # comma operator in array subscript expression (warning is issued incorrectly)
        /wd4250 # 'class1' : inherits 'class2::member' via dominance (Boost.IOStreams)
        /W4     # warning level 4
        /MP)    # multicore compilation
    list(APPEND COMP_DEFS "_CRT_SECURE_NO_WARNINGS" "_SCL_SECURE_NO_WARNINGS")

    # Disable auto-linkage
    list(APPEND COMP_DEFS  "BOOST_ALL_NO_LIB")

    # Comment out the following line, if you do not have
    # VLD (http://vld.codeplex.com) installed
    #list(APPEND COMP_DEFS "JD_HAS_VLD")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
    # see http://stackoverflow.com/a/11004675/2128694
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
    endif ()
    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    else ()
        message(FATAL_ERROR "Your C++ compiler does not support C++11.")
    endif ()
endif ()

if (WIN32)
    find_package(SFML REQUIRED system-s graphics-s window-s audio-s main)
else ()
    find_package(SFML REQUIRED system graphics window audio)
endif ()
find_package(PhysFS REQUIRED)
find_package(ZLIB REQUIRED)
set(Boost_USE_STATIC_LIBS    ON)
set(Boost_USE_MULTITHREADED  ON)
set(Boost_USE_STATIC_RUNTIME OFF)
# Cannot just append "filesystem" to the find_package line:
# somehow FindBoost.cmake fails to recognize it.
set(Boost_FIND_COMPONENTS system filesystem locale)
find_package(Boost REQUIRED system filesystem locale)
include_directories(
    "${CMAKE_SOURCE_DIR}/src"
    ${SFML_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    ${LUABIND_INCLUDE_DIR}
    ${PHYSFS_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS})
add_executable(jd WIN32 ${JD_HEADERS} ${JD_SOURCES})
set_target_properties(jd PROPERTIES COMPILE_DEFINITIONS "${COMP_DEFS}")
target_link_libraries(jd
    ${SFML_LIBRARIES}
    ${LUA_LIBRARY}
    ${LUABIND_LIBRARY}
    ${PHYSFS_LIBRARY}
    ${ZLIB_LIBRARY}
    ${Boost_LIBRARIES})

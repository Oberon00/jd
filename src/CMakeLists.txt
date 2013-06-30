# Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
# This file is subject to the terms of the BSD 2-Clause License.
# See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

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
source_group("Services" FILES ${SVC_SOURCES} ${SVC_HEADERS})


set(COMPSYS_HEADERS
    compsys/Component.hpp
    compsys/MetaComponent.hpp
    compsys/BasicMetaComponent.hpp
    compsys/ComponentRegistry.hpp
    compsys/Entity.hpp)

set(COMPSYS_SOURCES
    compsys/MetaComponent.cpp
    compsys/Component.cpp
    compsys/ComponentRegistry.cpp
    compsys/Entity.cpp)

source_group("Component System" FILES ${COMPSYS_SOURCES} ${COMPSYS_HEADERS})


set(COMP_HEADERS
    comp/PositionComponent.hpp
    comp/TileCollisionComponent.hpp
    comp/TilePositionComponent.hpp
    comp/RectCollisionComponent.hpp)

set(COMP_SOURCES
    comp/PositionComponent.cpp
    comp/TileCollisionComponent.cpp
    comp/TilePositionComponent.cpp)
   
source_group("Components" FILES ${COMP_SOURCES} ${COMP_HEADERS})


set(LUAEXPORT_HEADERS
    luaexport/ExportThis.hpp
    luaexport/LuaExportMacros.hpp
    luaexport/LuaFunction.hpp
    luaexport/LuaEventHelpers.hpp
    luaexport/SfBaseTypes.hpp
    luaexport/sharedPtrConverter.hpp
    luaexport/container.hpp
    luaexport/resources.hpp)

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

source_group("LuaExport" FILES ${LUAEXPORT_SOURCES} ${LUAEXPORT_HEADERS})


set(RESSYS_HEADERS
    ressys/resourceLoaders.hpp
    ressys/resfwd.hpp
    ressys/AutoResource.hpp
    ressys/AutoTexture.hpp
    ressys/AutoFont.hpp
    ressys/AutoSoundBuffer.hpp
    ressys/ResourceManager.hpp
    ressys/ResourceManager.inl
    ressys/VFileFont.hpp
    ressys/VFileMusic.hpp)

set(RESSYS_SOURCES
    ressys/resourceLoaders.cpp)

source_group("Resource System" FILES ${RESSYS_SOURCES} ${RESSYS_HEADERS})


set(COLLISION_HEADERS
    collision/Collisions.hpp
    collision/TileCollideableGroup.hpp
    collision/RectCollideableGroup.hpp)

set(COLLISION_SOURCES
    collision/Collisions.cpp
    collision/TileCollideableGroup.cpp
    collision/RectCollideableGroup.cpp)
    
source_group("Collisions" FILES ${COLLISION_SOURCES} ${COLLISION_HEADERS})
    

set(JD_HEADERS
    encoding.hpp
    MapInfo.hpp
    exceptions.hpp
    sfKeyCodes.hpp
    jdConfig.hpp
    cmdline.hpp
    WeakRef.hpp
    Tilemap.hpp
    TransformGroup.hpp
    Logfile.hpp
    sfUtil.hpp
    State.hpp
    base64.hpp
    luaUtils.hpp
    ${SVC_HEADERS}
    ${COMPSYS_HEADERS}
    ${COMP_HEADERS}
    ${LUAEXPORT_HEADERS}
    ${RESSYS_HEADERS}
    ${COLLISION_HEADERS})

set(JD_SOURCES
    encoding.cpp
    MapInfo.cpp
    main.cpp
    luaUtils.cpp
    jdConfig.cpp
    Tilemap.cpp
    TransformGroup.cpp
    Logfile.cpp
    base64.cpp
    sfUtil.cpp
    Resources.rc
    ${SVC_SOURCES}
    ${COMPSYS_SOURCES}
    ${COMP_SOURCES}
    ${LUAEXPORT_SOURCES}
    ${RESSYS_SOURCES}
    ${COLLISION_SOURCES})

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
find_package(Lua52 REQUIRED)
find_package(Luabind REQUIRED)
find_package(ssig REQUIRED)
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
    ${LUABIND_INCLUDE_DIRS}
    ${PHYSFS_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${SSIG_INCLUDE_DIRS})
add_executable(jd WIN32 ${JD_HEADERS} ${JD_SOURCES})
set_target_properties(jd PROPERTIES COMPILE_DEFINITIONS "${COMP_DEFS}")
target_link_libraries(jd
    ${SFML_LIBRARIES}
    ${LUA_LIBRARIES}
    ${LUABIND_LIBRARIES}
    ${PHYSFS_LIBRARY}
    ${ZLIB_LIBRARY}
    ${Boost_LIBRARIES})

install(TARGETS jd RUNTIME DESTINATION bin)

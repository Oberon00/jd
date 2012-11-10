set(SVC_HEADERS
    svc/Mainloop.hpp
    svc/StateManager.hpp
    svc/LuaVm.hpp
    svc/FileSystem.hpp
    svc/EventDispatcher.hpp
    svc/DrawService.hpp
    svc/Configuration.hpp
    svc/ServiceLocator.hpp
	svc/Timer.hpp)    
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
	svc/Timer.cpp)
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
    luaexport/Geometry.hpp
    luaexport/sharedPtrConverter.hpp
    luaexport/container.hpp)
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
    luaexport/SfVector.cpp
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
	luaexport/RectCollisionComponentMeta.cpp)
source_group("LuaExport Sources" FILES ${LUAEXPORT_SOURCES})


set(JD_HEADERS
    MapInfo.hpp
    exceptions.hpp
    resourceLoaders.hpp
    resfwd.hpp
    AutoResource.hpp
    AutoTexture.hpp
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
    ${SVC_HEADERS}
    ${COMPSYS_HEADERS}
    ${COMP_HEADERS}
    ${LUAEXPORT_HEADERS})
   
set(JD_SOURCES
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
    ${SVC_SOURCES}
    ${COMPSYS_SOURCES}
    ${COMP_SOURCES}
    ${LUAEXPORT_SOURCES})

set(COMP_DEFS "")

if (MSVC)
    add_definitions(
	/wd4251 # disable warnings about undefined DLL interfaces (luabind)
    /wd4709 # comma operator in array subscript expression (warning is issued incorrectly)
	/wd4250 # 'class1' : inherits 'class2::member' via dominance (Boost.IOStreams)
	/W4     # warning level 4
	/MP)    # multicore compilation
    list(APPEND COMP_DEFS "_CRT_SECURE_NO_WARNINGS" "_SCL_SECURE_NO_WARNINGS")
    
    # Comment out the following line, if you do not have 
	# VLD (http://vld.codeplex.com) installed
    list(APPEND COMP_DEFS "JD_HAS_VLD")
endif()

find_package(SFML REQUIRED system graphics window)
find_package(PhysFs REQUIRED)
find_package(zlib REQUIRED)
include_directories(
    "${CMAKE_SOURCE_DIR}/src"
    ${SFML_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    ${LUABIND_INCLUDE_DIR}
    ${PHYSFS_INCLUDE_DIR}
    ${SSIG_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR})
add_executable(jd ${JD_HEADERS} ${JD_SOURCES})
set_target_properties(jd PROPERTIES COMPILE_DEFINITIONS "${COMP_DEFS}")
target_link_libraries(jd
    ${SFML_LIBRARIES}
    ${LUA_LIBRARY}
    ${LUABIND_LIBRARY}
    ${PHYSFS_LIBRARY}
    ${ZLIB_LIBRARY})


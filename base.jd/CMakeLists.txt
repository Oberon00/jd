set (SRCS
    lua/lib/compsys.lua
    lua/lib/evt.lua
    lua/lib/maploader.lua
    lua/lib/oo.lua
    lua/lib/persistence.lua
    lua/lib/tabutil.lua
    lua/lib/text.lua
    lua/lib/util.lua
    lua/lib/vecutil.lua)
    
set (basejdout "${CMAKE_CURRENT_BINARY_DIR}/base.jd")

add_custom_command(
    OUTPUT ${basejdout}
    COMMAND python "${CMAKE_CURRENT_LIST_DIR}/mkzip.py"
                   ${basejdout}
                   ${SRCS}
    DEPENDS ${SRCS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Zipping base.jd."
    VERBATIM)

add_custom_target(basejd ALL
    DEPENDS ${basejdout}
    SOURCES ${SRCS})

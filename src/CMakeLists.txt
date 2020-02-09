add_subdirectory(fs-polyfill)

include(warnings)
include(dir_filters)

add_subdirectory(common)
add_subdirectory(plugin)

set(MGPS_SOURCES
    mgps/api.hh
    mgps/export.hh
    mgps/library.cc
    mgps/library.hh
    mgps/media_file.hh
    mgps/plugins/host/dynamic_plugin.cc
    mgps/plugins/host/dynamic_plugin.hh
    mgps/plugins/host/host.cc
    mgps/plugins/host/host.hh
    mgps/plugins/host/loader.hh
    mgps/plugins/plugin_interface.cc
    mgps/plugins/plugin_interface.hh
    mgps/track/point.cc
    mgps/track/polyline.cc
    mgps/track/trace.cc
    mgps/trip.cc
    mgps/trip.hh
)

if (UNIX)
    list(APPEND MGPS_SOURCES
        mgps/plugins/host/loader.posix.cc
    )
else()
    list(APPEND MGPS_SOURCES
        mgps/plugins/host/loader.win32.cc
    )
endif()

add_library(mgps SHARED ${MGPS_SOURCES})

target_include_directories(mgps PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(mgps PUBLIC fs-polyfill mgps-common PRIVATE mgps-plugin)
target_compile_definitions(mgps PRIVATE MGPS_COMPILED=1)
if (NOT MSVC)
    target_compile_options(mgps PRIVATE -fvisibility=hidden)
endif()

if (UNIX)
    target_link_libraries(mgps PUBLIC dl)
endif()

dir_filters(MGPS_SOURCES)
set_target_properties(mgps PROPERTIES
    FOLDER ${FOLDER_CORE}
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS mgps 
    ARCHIVE COMPONENT dev
    OBJECTS COMPONENT dev
    LIBRARY
        COMPONENT library
        NAMELINK_COMPONENT dev
    RUNTIME COMPONENT library)
install(DIRECTORY mgps/
    COMPONENT dev
    DESTINATION include/mgps
    FILES_MATCHING PATTERN "*.hh")
        
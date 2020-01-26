add_subdirectory(fs-polyfill)

include(warnings)
include(dir_filters)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.in.hh" "${CMAKE_CURRENT_BINARY_DIR}/include/mgps/version.hh" @ONLY)

set(MGPS_SOURCES
    mgps/clocks.cc
    mgps/clocks.hh
    mgps/cstdio.cc
    mgps/cstdio.hh
    mgps/isom.cc
    mgps/isom.hh
    mgps/library.cc
    mgps/library.hh
    mgps/media_file.cc
    mgps/media_file.hh
    mgps/plugins/client.hh
    mgps/plugins/host/dynamic_plugin.cc
    mgps/plugins/host/dynamic_plugin.hh
    mgps/plugins/host/host.cc
    mgps/plugins/host/host.hh
    mgps/plugins/host/loader.hh
    mgps/plugins/plugin_interface.cc
    mgps/plugins/plugin_interface.hh
    mgps/track/boundary.cc
    mgps/track/boundary.hh
    mgps/track/coordinate.cc
    mgps/track/coordinate.hh
    mgps/track/point.cc
    mgps/track/point.hh
    mgps/track/polyline.cc
    mgps/track/polyline.hh
    mgps/track/speed.cc
    mgps/track/speed.hh
    mgps/track/trace.cc
    mgps/track/trace.hh
    mgps/trip.cc
    mgps/trip.hh
    mgps/video/media_clip.cc
    mgps/video/media_clip.hh
    mgps/video/playlist.cc
    mgps/video/playlist.hh
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

add_library(mgps STATIC ${MGPS_SOURCES})

target_include_directories(mgps PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(mgps date::date fs-polyfill)

dir_filters(MGPS_SOURCES)
set_target_properties(mgps PROPERTIES
    FOLDER ${FOLDER_CORE}
    POSITION_INDEPENDENT_CODE ON)

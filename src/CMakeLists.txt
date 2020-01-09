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
    mgps/video/file.cc
    mgps/video/file.hh
    mgps/video/gap.cc
    mgps/video/gap.hh
    mgps/video/playlist.cc
    mgps/video/playlist.hh
)
add_library(mgps STATIC ${MGPS_SOURCES})

target_include_directories(mgps PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(mgps date::date)

if (NOT (MSVC OR (CMAKE_SYSTEM_NAME STREQUAL "Android")))
	target_link_libraries(mgps stdc++fs)
endif()

dir_filters(MGPS_SOURCES)

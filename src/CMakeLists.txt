include(warnings)
include(dir_filters)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.in.hh" "${CMAKE_CURRENT_BINARY_DIR}/include/mgps/version.hh" @ONLY)

set(MGPS_SOURCES
    mgps/clocks.cc
    mgps/cstdio.cc
    mgps/drive.cc
    mgps/clocks.hh
    mgps/cstdio.hh
    mgps/drive.hh
    mgps/isom.hh
    mgps/track/boundary.hh
    mgps/track/coordinate.hh
    mgps/track/point.hh
    mgps/track/polyline.hh
    mgps/track/speed.hh
    mgps/track/trace.hh
    mgps/video/file.hh
    mgps/video/gap.hh
    mgps/video/playlist.hh
    mgps/isom.cc
    mgps/track/boundary.cc
    mgps/track/coordinate.cc
    mgps/track/point.cc
    mgps/track/polyline.cc
    mgps/track/speed.cc
    mgps/track/trace.cc
    mgps/video/file.cc
    mgps/video/gap.cc
    mgps/video/playlist.cc
)
add_library(mgps STATIC ${MGPS_SOURCES})

target_include_directories(mgps PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(mgps date::date)

if (NOT MSVC)
	target_link_libraries(mgps stdc++fs)
endif()

dir_filters(MGPS_SOURCES)

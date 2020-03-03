configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.in.hh" "${CMAKE_CURRENT_BINARY_DIR}/include/mgps/version.hh" @ONLY)

set(MGPS_COMMON_SOURCES_
    mgps/clip.hh
    mgps/clocks.hh
    mgps/platform_export.hh
    mgps/track/boundary.hh
    mgps/track/coordinate.hh
    mgps/track/point.hh
    mgps/track/polyline.hh
    mgps/track/speed.hh
    mgps/track/trace.hh
    mgps/video/media_clip.hh
    mgps/video/playlist.hh
)

foreach(SRC ${MGPS_COMMON_SOURCES_})
    list(APPEND MGPS_COMMON_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${SRC}")
endforeach()

list(APPEND MGPS_COMMON_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/include/mgps/version.hh")

add_library(mgps-common INTERFACE IMPORTED GLOBAL)
target_sources(mgps-common INTERFACE ${MGPS_COMMON_SOURCES})
target_include_directories(mgps-common INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(mgps-common INTERFACE fs-polyfill CONAN_PKG::date)
dir_filters(MGPS_COMMON_SOURCES)

# Test builds will need the mgps, but not mgps' install targets
if (NOT MGPS_BUILD_TESTS_ONLY)
    install(DIRECTORY mgps/
        COMPONENT dev
        DESTINATION include/mgps
        FILES_MATCHING PATTERN "*.hh")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/mgps/version.hh
        COMPONENT dev
        DESTINATION include/mgps)
endif()
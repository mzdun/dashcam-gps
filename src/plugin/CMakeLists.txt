set(MGPS_PLUGIN_SOURCES
    mgps/plugin/cstdio.cc
    mgps/plugin/cstdio.hh
    mgps/plugin/isom.cc
    mgps/plugin/isom.hh
    mgps/plugin/plugin.hh
)

add_library(mgps-plugin STATIC ${MGPS_PLUGIN_SOURCES})

target_include_directories(mgps-plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(mgps-plugin PUBLIC mgps-common)

dir_filters(MGPS_PLUGIN_SOURCES)
set_target_properties(mgps-plugin PROPERTIES
    FOLDER ${FOLDER_CORE}
    POSITION_INDEPENDENT_CODE ON)

if (NOT MSVC)
    target_compile_options(mgps-plugin PRIVATE -fvisibility=hidden)
endif()

if (NOT MGPS_BUILD_TESTS_ONLY)
    install(TARGETS mgps-plugin COMPONENT dev)
    install(DIRECTORY mgps/
        COMPONENT dev
        DESTINATION include/mgps
        FILES_MATCHING PATTERN "*.hh")
endif()
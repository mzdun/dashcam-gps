include(warnings)
include(dir_filters)

set(PLUGIN_SOURCES
    include/mgps-70mai/70mai.hh
    include/mgps-70mai/plugin.hh
    src/70mai.cc
    src/plugin.cc
)
add_library(mgps-70mai STATIC ${PLUGIN_SOURCES})

target_include_directories(mgps-70mai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mgps-70mai PUBLIC mgps PRIVATE CONAN_PKG::CTRE)
set_target_properties(mgps-70mai PROPERTIES
    FOLDER ${FOLDER_EXTRAS}
    POSITION_INDEPENDENT_CODE ON)

dir_filters(PLUGIN_SOURCES)

set(DYNAMIC_PLUGIN_SOURCES
    src/exports.cc
)
add_library(mgps-plugin-70mai SHARED ${DYNAMIC_PLUGIN_SOURCES})

target_include_directories(mgps-plugin-70mai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mgps-plugin-70mai PRIVATE mgps-70mai)
set_target_properties(mgps-plugin-70mai PROPERTIES
    FOLDER ${FOLDER_EXTRAS}
    POSITION_INDEPENDENT_CODE ON
	VERSION ${PROJECT_VERSION}
    PREFIX "")

dir_filters(DYNAMIC_PLUGIN_SOURCES)

if (NOT ANDROID)
    include(application)
    plugindir(mgps-plugin-70mai)
endif()

install(TARGETS mgps-plugin-70mai DESTINATION bin/mgps-plugin)

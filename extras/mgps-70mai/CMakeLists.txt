include(warnings)
include(dir_filters)

set(PLUGIN_SOURCES
    include/mgps-70mai/70mai.hh
    include/mgps-70mai/plugin.hh
    src/70mai.cc
    src/exports.cc
    src/plugin.cc
)
dir_filters(PLUGIN_SOURCES)

add_library(mgps-plugin-70mai SHARED ${PLUGIN_SOURCES})
target_include_directories(mgps-plugin-70mai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mgps-plugin-70mai PRIVATE mgps CONAN_PKG::CTRE)
set_target_properties(mgps-plugin-70mai PROPERTIES
    FOLDER ${FOLDER_EXTRAS}
    POSITION_INDEPENDENT_CODE ON
    PREFIX "")

install(TARGETS mgps-plugin-70mai
    DESTINATION lib/mgps
    COMPONENT plugin-70mai)

include(warnings)
include(dir_filters)
include(mgps-plugin)

plugin_output_dirs_prepare()

set(PLUGIN_SOURCES
    70mai.hh
    70mai.cc
    exports.cc
)
dir_filters(PLUGIN_SOURCES)

add_library(mgps-plugin-70mai SHARED ${PLUGIN_SOURCES})
target_include_directories(mgps-plugin-70mai PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(mgps-plugin-70mai PRIVATE IMPLEMENTING_PLUGIN)
if (NOT MSVC)
    target_compile_options(mgps-plugin-70mai PRIVATE -fvisibility=hidden)
endif()
target_link_libraries(mgps-plugin-70mai PRIVATE mgps-plugin CONAN_PKG::CTRE)
set_target_properties(mgps-plugin-70mai PROPERTIES
    FOLDER ${FOLDER_EXTRAS}
    POSITION_INDEPENDENT_CODE ON
    PREFIX "")

set_plugin_output_directories(mgps-plugin-70mai)

install(TARGETS mgps-plugin-70mai
    LIBRARY
        DESTINATION lib/mgps
        COMPONENT plugin-70mai
    RUNTIME
        DESTINATION bin/mgps
        COMPONENT plugin-70mai)

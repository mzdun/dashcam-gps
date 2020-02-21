include(warnings)
include(dir_filters)
include(mgps-plugin)

set(LIB70MAI_SOURCES
    70mai.hh
    70mai.cc
    api.cc
    api.hh
)

plugin_output_dirs_prepare()

set(PLUGIN_SOURCES
    exports.cc
)
dir_filters(LIB70MAI_SOURCES)
dir_filters(PLUGIN_SOURCES)

add_library(70mai STATIC ${LIB70MAI_SOURCES})
target_include_directories(70mai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(70mai PUBLIC mgps-plugin CONAN_PKG::CTRE)
set_target_properties(70mai PROPERTIES
    FOLDER ${FOLDER_EXTRAS}
    POSITION_INDEPENDENT_CODE ON)

add_library(mgps-plugin-70mai SHARED ${PLUGIN_SOURCES})
target_compile_definitions(mgps-plugin-70mai PRIVATE IMPLEMENTING_PLUGIN)
target_link_libraries(mgps-plugin-70mai PRIVATE 70mai)
set_target_properties(mgps-plugin-70mai PROPERTIES
    FOLDER ${FOLDER_EXTRAS}
    POSITION_INDEPENDENT_CODE ON
    PREFIX "")

set_plugin_output_directories(mgps-plugin-70mai)

if (NOT MSVC)
    target_compile_options(70mai PRIVATE -fvisibility=hidden)
    target_compile_options(mgps-plugin-70mai PRIVATE -fvisibility=hidden)
endif()


install(TARGETS mgps-plugin-70mai
    LIBRARY
        DESTINATION lib/mgps
        COMPONENT plugin-70mai
    RUNTIME
        DESTINATION bin/mgps
        COMPONENT plugin-70mai)

if (MGPS_BUILD_TESTS)
    add_executable(70mai-tests 70mai_filename_info_unittests.cc)
    target_include_directories(70mai-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(70mai-tests PRIVATE 70mai CONAN_PKG::gtest)
    set_target_properties(70mai-tests PROPERTIES
        FOLDER ${FOLDER_EXTRAS})

    add_test(NAME 70mai WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND bin/70mai-tests --gtest_output=xml:testing-results/70mai-tests.xml)

endif()

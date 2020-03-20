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

if (NOT MGPS_BUILD_TESTS_ONLY)
    add_library(mgps-plugin-70mai SHARED ${PLUGIN_SOURCES})
    target_compile_definitions(mgps-plugin-70mai PRIVATE IMPLEMENTING_PLUGIN)
    target_link_libraries(mgps-plugin-70mai PRIVATE 70mai)
    set_target_properties(mgps-plugin-70mai PROPERTIES
        FOLDER ${FOLDER_EXTRAS}
        POSITION_INDEPENDENT_CODE ON
        PREFIX "")
endif()

if (MGPS_BUILD_FUZZ)
    add_executable(fuzzer-70mai-boxes fuzzer/boxes.cc)
    target_include_directories(fuzzer-70mai-boxes PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(fuzzer-70mai-boxes PRIVATE 70mai CONAN_PKG::CTRE)
    set_target_properties(fuzzer-70mai-boxes PROPERTIES
        FOLDER ${FOLDER_EXTRAS})
    install(TARGETS fuzzer-70mai-boxes COMPONENT fuzzer)
    install(DIRECTORY fuzzer/boxes-in/
        COMPONENT fuzzer
        DESTINATION share/mgps/fuzzer/boxes-in/
    )
endif()

if (NOT MGPS_BUILD_TESTS_ONLY)
    set_plugin_output_directories(mgps-plugin-70mai)
endif()

if (NOT MSVC)
    target_compile_options(70mai PRIVATE -fvisibility=hidden)
    if (NOT MGPS_BUILD_TESTS_ONLY)
        target_compile_options(mgps-plugin-70mai PRIVATE -fvisibility=hidden)
    endif()
endif()


if (NOT MGPS_BUILD_TESTS_ONLY)
    install(TARGETS mgps-plugin-70mai
        LIBRARY
            DESTINATION lib/mgps
            COMPONENT plugin-70mai
        RUNTIME
            DESTINATION bin/mgps
            COMPONENT plugin-70mai)
endif()

if (MGPS_BUILD_TESTS)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/70mai_api_unittests.hh.in" "${CMAKE_CURRENT_BINARY_DIR}/70mai_api_unittests.hh" @ONLY)
    add_executable(70mai-tests fuzzer/data/xxd_fs_data_unittests.cc exports.cc 70mai_filename_info_unittests.cc 70mai_api_unittests.cc)
    target_compile_definitions(70mai-tests PRIVATE IMPLEMENTING_PLUGIN)
    target_include_directories(70mai-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(70mai-tests PRIVATE 70mai mgps-static CONAN_PKG::gtest)
    set_target_properties(70mai-tests PROPERTIES
        FOLDER ${FOLDER_EXTRAS})

    if (NOT DEFINED ENV{PROJECT_VERSION_BUILD_RELEASE} OR DEFINED ENV{PROJECT_VERSION_BUILD_RELEASE_TESTS})
        install(TARGETS 70mai-tests COMPONENT tests)
    endif()

    add_test(NAME 70mai WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND bin/70mai-tests --gtest_output=xml:testing-results/70mai-tests.xml)
endif()

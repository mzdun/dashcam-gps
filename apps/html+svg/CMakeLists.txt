include(warnings)

add_executable(html+svg main.cc svg.cc svg.hh debug.cc debug.hh)
target_link_libraries(html+svg PRIVATE mgps CONAN_PKG::fmt)
set_target_properties(html+svg PROPERTIES FOLDER ${FOLDER_APPS})

install(TARGETS html+svg COMPONENT tools)

if (MGPS_BUILD_TESTS)
    add_executable(html+svg-tests debug.cc debug.hh debug_fmt_unittests.cc)
    target_include_directories(html+svg-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(html+svg-tests PRIVATE mgps-common CONAN_PKG::fmt CONAN_PKG::gtest)
    set_target_properties(html+svg-tests PROPERTIES FOLDER ${FOLDER_APPS})

    add_test(NAME html+svg WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND bin/html+svg-tests --gtest_output=xml:testing-results/html+svg-tests.xml)

endif()

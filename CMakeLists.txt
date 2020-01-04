cmake_minimum_required(VERSION 3.10)
project(dashcam-gps
    DESCRIPTION "Library and player for movie clips with accompanying GPS info"
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

# The date library needs help with detecting compilers supporting
# uncaught_exceptions (plural); the test for __cplusplus greater than
# March 2017 does not work for VC and gcc...
add_definitions(-DHAS_UNCAUGHT_EXCEPTIONS=1)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(extras)
add_subdirectory(src/qt5-mgps-player qt5-mgps-player)
add_subdirectory(examples)

set_target_properties(mgps PROPERTIES FOLDER "core")
set_target_properties(mgps-70mai PROPERTIES FOLDER "core")
set_target_properties(mgps-player PROPERTIES FOLDER "apps")
set_target_properties(html+svg PROPERTIES FOLDER "apps")
set_target_properties(Continuous PROPERTIES FOLDER "external/ctre")
set_target_properties(Experimental PROPERTIES FOLDER "external/ctre")
set_target_properties(Nightly PROPERTIES FOLDER "external/ctre")
set_target_properties(NightlyMemoryCheck PROPERTIES FOLDER "external/ctre")

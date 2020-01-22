cmake_minimum_required(VERSION 3.10)
project(dashcam-gps
    DESCRIPTION "Library and player for movie clips with accompanying GPS info"
    VERSION 0.1.0.0
    LANGUAGES CXX)

set(PROJECT_VERSION_STABILITY "-alpha") # or "-beta", or "-rc1", or "-rc2", or ""

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Predefined Targets")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# The date library needs help with detecting compilers supporting
# uncaught_exceptions (plural); the test for __cplusplus greater than
# March 2017 does not work for VC and gcc...
add_definitions(-DHAS_UNCAUGHT_EXCEPTIONS=1)

set(FOLDER_APPS "Apps")
set(FOLDER_APPS_EXTRAS "Apps/Extras")
set(FOLDER_CORE "Core")
set(FOLDER_EXTRAS "Core/Extras")
set(FOLDER_EXTERNAL "External")

if (NOT ANDROID)
    find_package(JNI)
endif()

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(extras)
add_subdirectory(apps)

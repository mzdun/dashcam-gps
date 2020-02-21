cmake_minimum_required(VERSION 3.10)
project(dashcam-gps
    DESCRIPTION "Library and player for movie clips with accompanying GPS info"
    VERSION 0.4.3
    LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(PROJECT_VERSION_STABILITY "-alpha" # or "-beta", or "-rc.1", or "-rc.2", or ""
    CACHE STRING "" FORCE)

include(semver_meta)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Predefined Targets")

# The date library needs help with detecting compilers supporting
# uncaught_exceptions (plural); the test for __cplusplus greater than
# March 2017 does not work for VC and gcc...
add_definitions(-DHAS_UNCAUGHT_EXCEPTIONS=1)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(FOLDER_APPS "Apps")
set(FOLDER_APPS_EXTRAS "Apps/Extras")
set(FOLDER_CORE "Core")
set(FOLDER_EXTRAS "Core/Extras")

option(MGPS_BUILD_TOOLS "If selected, will build html+svg and listing" OFF)
option(MGPS_BUILD_QT5 "If selected, will build qt5-player; needs Qt5_DIR variable" OFF)
option(MGPS_BUILD_JNI "If selected, will try to build Java/JNI code even without Android" OFF)
option(MGPS_BUILD_70MAI "If selected, will build mgps-plugin-70mai" OFF)
option(MGPS_BUILD_TESTS "If selected, will build various tests" OFF)
option(MGPS_PACK_COMPONENTS "If selected, will pack components to the archives of their own" ON)

# Taken from https://github.com/HowardHinnant/date
function(print_option OPT)
    if (NOT DEFINED PRINT_OPTION_CURR_${OPT} OR (NOT PRINT_OPTION_CURR_${OPT} STREQUAL ${OPT}))
        set(PRINT_OPTION_CURR_${OPT} ${${OPT}} CACHE BOOL "" FORCE)
        mark_as_advanced(PRINT_OPTION_CURR_${OPT})
        message("# mgps: ${OPT} ${${OPT}}")
    endif( )
endfunction( )

foreach(OPT MGPS_PACK_COMPONENTS MGPS_BUILD_TOOLS MGPS_BUILD_QT5 MGPS_BUILD_70MAI MGPS_BUILD_TESTS)
	print_option(${OPT})
endforeach()
if (NOT ANDROID)
	print_option(MGPS_BUILD_JNI)
endif()

if (NOT ANDROID AND MGPS_BUILD_JNI)
    find_package(JNI)
endif()

if (MGPS_BUILD_TESTS)
    enable_testing()
endif()

add_subdirectory(src)
add_subdirectory(extras)
add_subdirectory(apps)

include(packing)

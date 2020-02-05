# The polyfill was inspired by https://github.com/AshampooSystems/boden
# The source code itself seems to come from libc++

include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "-std=c++17")
CHECK_CXX_SOURCE_COMPILES("#include <filesystem>\nint main() { std::filesystem::space(\"/\"); }" HAS_STD_FILESYSTEM)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU") 
    CHECK_CXX_SOURCE_COMPILES("#include <__config>\nint main() { }" HAS_LIBCXX)
endif()

if (NOT HAS_STD_FILESYSTEM)
    if (HAS_LIBCXX)
        set(CMAKE_REQUIRED_LIBRARIES c++fs)
        CHECK_CXX_SOURCE_COMPILES("#include <filesystem>\nint main() { std::filesystem::space(\"/\"); }" HAS_STD_FILESYSTEM_CXXFS)
    else()
        set(CMAKE_REQUIRED_LIBRARIES stdc++fs)
        CHECK_CXX_SOURCE_COMPILES("#include <filesystem>\nint main() { std::filesystem::space(\"/\"); }" HAS_STD_FILESYSTEM_STDCXXFS)
    endif()
endif()

set(FILESYSTEM_BACKING "NOTFOUND")

if (HAS_STD_FILESYSTEM OR HAS_STD_FILESYSTEM_STDCXXFS OR HAS_STD_FILESYSTEM_CXXFS)
    add_library(fs-polyfill INTERFACE IMPORTED GLOBAL)
    
    if (HAS_LIBCXX AND HAS_STD_FILESYSTEM_CXXFS)
        set(FILESYSTEM_BACKING "libc++fs")
        target_link_libraries(fs-polyfill INTERFACE c++fs)
    elseif (NOT HAS_LIBCXX AND HAS_STD_FILESYSTEM_STDCXXFS)
        set(FILESYSTEM_BACKING "libstdc++fs")
        target_link_libraries(fs-polyfill INTERFACE stdc++fs)
    elseif (MSVC)
        set(FILESYSTEM_BACKING "msvcrt")
    elseif (HAS_LIBCXX)
        set(FILESYSTEM_BACKING "libc++")
    else()
        set(FILESYSTEM_BACKING "libstdc++")
    endif()
elseif (ANDROID)
    set(FILESYSTEM_BACKING "libfs-polyfill (libc++fs)")
    add_library(fs-polyfill STATIC
            operations.cpp
            directory_iterator.cpp
            filesystem_common.h
            int128_builtins.cpp)
    set_target_properties(fs-polyfill PROPERTIES
        FOLDER ${FOLDER_APPS_EXTRAS}
        POSITION_INDEPENDENT_CODE ON)
    install(TARGETS fs-polyfill)
endif()

if(FILESYSTEM_BACKING)
    if (NOT DEFINED PRINT_OPTION_CURR_FILESYSTEM_BACKING OR (NOT PRINT_OPTION_CURR_FILESYSTEM_BACKING STREQUAL FILESYSTEM_BACKING))
        set(PRINT_OPTION_CURR_FILESYSTEM_BACKING ${FILESYSTEM_BACKING} CACHE BOOL "" FORCE)
        mark_as_advanced(PRINT_OPTION_CURR_FILESYSTEM_BACKING)
        message(STATUS "<filesystem> implemented with ${FILESYSTEM_BACKING}")
    endif( )
else()
    message(FATAL_ERROR "<filesystem> implementation is missing")
endif()


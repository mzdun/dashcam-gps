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

if (HAS_STD_FILESYSTEM OR HAS_STD_FILESYSTEM_STDCXXFS OR HAS_STD_FILESYSTEM_CXXFS)
    add_library(droid++fs INTERFACE IMPORTED GLOBAL)
    
    if (HAS_STD_FILESYSTEM_CXXFS)
        message(STATUS "<filesystem> implemented with libc++fs")
        target_link_libraries(droid++fs INTERFACE c++fs)
    elseif (HAS_STD_FILESYSTEM_STDCXXFS)
        message(STATUS "<filesystem> implemented with libstdc++fs")
        target_link_libraries(droid++fs INTERFACE stdc++fs)
    elseif (MSVC)
        message(STATUS "<filesystem> implemented with msvcrt")
    elseif (HAS_LIBCXX)
        message(STATUS "<filesystem> implemented with libc++")
    else()
        message(STATUS "<filesystem> implemented with libstdc++")
    endif()
else()
    message(STATUS "<filesystem> implemented with libdroid++fs (libc++fs)")
    add_library(droid++fs STATIC
            operations.cpp
            directory_iterator.cpp
            filesystem_common.h
            int128_builtins.cpp)
    set_target_properties(droid++fs PROPERTIES
        FOLDER ${FOLDER_APPS_EXTRAS}
        POSITION_INDEPENDENT_CODE ON)
endif()


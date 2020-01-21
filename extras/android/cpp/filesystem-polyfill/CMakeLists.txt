# The polyfill was inspired by https://github.com/AshampooSystems/boden
# The source code itself seems to come from libc++

include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "-std=c++17")
CHECK_CXX_SOURCE_COMPILES("#include <filesystem>\nint main() { std::filesystem::space(\"/\"); }" HAS_STD_FILESYSTEM)

set(CMAKE_REQUIRED_LIBRARIES stdc++fs)
CHECK_CXX_SOURCE_COMPILES("#include <filesystem>\nint main() { std::filesystem::space(\"/\"); }" HAS_STD_FILESYSTEM_STDCXXFS)

set(CMAKE_REQUIRED_LIBRARIES c++fs)
CHECK_CXX_SOURCE_COMPILES("#include <filesystem>\nint main() { std::filesystem::space(\"/\"); }" HAS_STD_FILESYSTEM_CXXFS)

if (HAS_STD_FILESYSTEM OR HAS_STD_FILESYSTEM_STDCXXFS OR HAS_STD_FILESYSTEM_CXXFS)
    add_library(droid++fs INTERFACE IMPORTED GLOBAL)
    if (HAS_STD_FILESYSTEM_STDCXXFS)
        target_link_libraries(droid++fs INTERFACE stdc++fs)
    elseif (HAS_STD_FILESYSTEM_CXXFS)
        target_link_libraries(droid++fs INTERFACE c++fs)
    endif()
else()
    add_library(droid++fs STATIC
            operations.cpp
            directory_iterator.cpp
            filesystem_common.h
            int128_builtins.cpp)
    set_target_properties(droid++fs PROPERTIES
        FOLDER ${FOLDER_APPS_EXTRAS}
        POSITION_INDEPENDENT_CODE ON)
endif()


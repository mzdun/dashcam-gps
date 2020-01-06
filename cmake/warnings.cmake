if (MSVC)
    set(CMAKE_CXX_WARNING_LEVEL 4)

    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()

    if(CMAKE_CXX_FLAGS_DEFAULT MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS_DEFAULT "${CMAKE_CXX_FLAGS_DEFAULT}")
    endif()

    if(CMAKE_CXX_FLAGS_DEBUG MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    endif()

    if(CMAKE_CXX_FLAGS_RELEASE MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    endif()

    add_definitions(
        /D_CRT_SECURE_NO_WARNINGS=1
        /D_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING=1
        /permissive-
        /W4
        /w14242
        /w14254
        /w14263
        /w14265
        /w14287
        /we4289
        /w14296
        /w14311
        /w14545
        /w14546
        /w14547
        /w14549
        /w14555
        /w14619
        /w14640
        /w14826
        /w14905
        /w14906
        /w14928
        /w14946
    )
else()
    add_definitions(
        -std=c++17
        -Wall -Wextra
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_definitions(-fcolor-diagnostics) # -Wlifetime
    else()
        add_definitions(
            -fconcepts
            -fdiagnostics-color
            -Wmisleading-indentation
            -Wduplicated-cond
            -Wduplicated-branches
            -Wlogical-op
            -Wuseless-cast
        )
    endif()
endif()

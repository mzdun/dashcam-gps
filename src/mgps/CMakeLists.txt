add_library(mgps STATIC
    include/mgps/cstdio.hh
    include/mgps/isom.hh
    include/mgps/library.hh
    cstdio.cc
    isom.cc
    library.cc
	)

if (MSVC)
  add_definitions(
      /D_CRT_SECURE_NO_WARNINGS=1
      /D_SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING=1
      /permissive-
      # /W4
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

target_include_directories(mgps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mgps date::date)

if (NOT MSVC)
	target_link_libraries(mgps stdc++fs)
endif()

add_library(mgps STATIC
    include/mgps/cstdio.hh
    include/mgps/isom.hh
    include/mgps/library.hh
    cstdio.cc
    isom.cc
    library.cc
	)

target_include_directories(mgps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mgps date::date)

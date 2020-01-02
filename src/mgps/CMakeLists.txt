add_library(mgps STATIC
    include/mgps/library.hh
    library.cc
	)

target_include_directories(mgps PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mgps date::date)

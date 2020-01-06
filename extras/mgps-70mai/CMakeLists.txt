include(warnings)

add_library(mgps-70mai STATIC
    include/mgps-70mai/70mai.hh
    include/mgps-70mai/loader.hh
    src/70mai.cc
    src/loader.cc
)

target_include_directories(mgps-70mai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(mgps-70mai mgps ctre::ctre)

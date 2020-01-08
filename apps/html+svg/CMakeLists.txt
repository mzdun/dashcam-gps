include(warnings)

add_executable(html+svg main.cc svg.cc svg.hh debug.cc debug.hh)
target_link_libraries(html+svg mgps mgps-70mai)

include(application)
bindir(html+svg)

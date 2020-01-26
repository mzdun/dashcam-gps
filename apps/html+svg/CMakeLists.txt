include(warnings)

add_executable(html+svg main.cc svg.cc svg.hh debug.cc debug.hh)
target_link_libraries(html+svg PRIVATE mgps)
set_target_properties(html+svg PROPERTIES FOLDER ${FOLDER_APPS})

include(application)
bindir(html+svg)

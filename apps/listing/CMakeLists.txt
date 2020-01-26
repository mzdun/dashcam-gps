include(warnings)

add_executable(listing main.cc listing.cc listing.hh debug.cc debug.hh)
target_link_libraries(listing PRIVATE mgps)
set_target_properties(listing PROPERTIES FOLDER ${FOLDER_APPS})

include(application)
bindir(listing)

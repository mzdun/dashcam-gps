set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(android-player SHARED entrypoints.cc)
set_target_properties(android-player PROPERTIES OUTPUT_NAME "${PROJECT_NAME}-player-native" FOLDER ${FOLDER_APPS})
target_link_libraries(android-player mgps-droid)

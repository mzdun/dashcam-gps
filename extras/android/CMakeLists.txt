add_subdirectory(filesystem-polyfill)

include(warnings)
add_library(jni STATIC
    jni/binding_ref.cc
    jni/binding_ref.hh
    jni/env.cc
    jni/env.hh
    jni/field.cc
    jni/field.hh
    jni/fixed_string.hh
    jni/method.cc
    jni/method.hh
    jni/primitive.hh
    jni/ref.cc
    jni/ref.hh
    jni/type_base.cc
    jni/type_base.hh
    jni/type_info.cc
    jni/type_info.hh
)

target_include_directories(jni PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(jni PROPERTIES FOLDER ${FOLDER_APPS_EXTRAS})

if (ANDROID)
    find_library(ANDROID_LOG log REQUIRED)
    mark_as_advanced(ANDROID_LOG)
    message(STATUS "log: ${ANDROID_LOG}")

    target_link_libraries(jni PUBLIC ${ANDROID_LOG})
else()
    target_link_libraries(jni PUBLIC ${JNI_LIBRARIES})
    target_include_directories(jni PUBLIC ${JNI_INCLUDE_DIRS})
endif()

set(CLASS_SRC
    class/com/midnightbits/dashcam_gps_player/data/Library.cc
    class/com/midnightbits/dashcam_gps_player/data/Library.hh
    class/com/midnightbits/dashcam_gps_player/data/PACKAGE.hh
    class/com/midnightbits/dashcam_gps_player/PACKAGE.hh
    class/com/midnightbits/dashcam_gps_player/R.cc
    class/com/midnightbits/dashcam_gps_player/R.hh
    class/java/time/Duration.hh
    class/java/time/PACKAGE.hh
    class/java/util/ArrayList.hh
    class/java/util/Date.hh
    class/java/util/List.hh
    class/java/util/PACKAGE.hh
)
add_library(mgps-droid STATIC ${CLASS_SRC})
set_target_properties(mgps-droid PROPERTIES FOLDER ${FOLDER_APPS_EXTRAS})
target_link_libraries(mgps-droid PUBLIC jni mgps mgps-70mai droid++fs)
target_include_directories(mgps-droid PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/class)

include(dir_filters)
dir_filters(CLASS_SRC)

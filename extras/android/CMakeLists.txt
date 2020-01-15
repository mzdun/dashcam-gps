include(warnings)
add_library(jni STATIC
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

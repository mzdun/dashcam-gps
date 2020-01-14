include(warnings)
add_library(jni STATIC
    jni/class.cc
    jni/class.hh
    jni/env.cc
    jni/env.hh
    jni/field.cc
    jni/field.hh
    jni/method.cc
    jni/method.hh
    jni/object.cc
    jni/object.hh
    jni/refs.cc
    jni/refs.hh
    jni/type_info.cc
    jni/type_info.hh
)

target_include_directories(jni PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

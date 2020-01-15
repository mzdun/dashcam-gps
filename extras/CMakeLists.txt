add_subdirectory(mgps-70mai)

find_package(JNI)

if (ANDROID OR JNI_FOUND)
	add_subdirectory(android)
endif()
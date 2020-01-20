add_subdirectory(mgps-70mai)

if (ANDROID OR JNI_FOUND)
	add_subdirectory(android/src/main/cpp android)
endif()
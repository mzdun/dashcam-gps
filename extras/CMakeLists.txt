if (MGPS_BUILD_70MAI)
	add_subdirectory(mgps-70mai)
endif()

if (ANDROID OR JNI_FOUND)
	add_subdirectory(android)
endif()

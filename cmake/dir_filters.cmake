function(dir_filters SRCS)
    foreach(FILE ${${SRCS}}) 
        #convert source file to absolute
        get_filename_component(ABSOLUTE_PATH "${FILE}" ABSOLUTE)
        # Get the directory of the absolute source file
        get_filename_component(PARENT_DIR "${ABSOLUTE_PATH}" DIRECTORY)
        # Remove common directory prefix to make the group
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
        # Make sure we are using windows slashes
        string(REPLACE "/" "\\" GROUP "${GROUP}")
        # Group into "Source Files" and "Header Files"
        if ("${FILE}" MATCHES ".*\\.c")
            set(GROUP "Source Files${GROUP}")
        elseif("${FILE}" MATCHES ".*\\.h")
            set(GROUP "Header Files${GROUP}")
        else()
            set(GROUP "Other Files${GROUP}")
        endif()
        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endfunction()
add_subdirectory(date)
add_subdirectory(ctre)

set_target_properties(Continuous Experimental Nightly NightlyMemoryCheck PROPERTIES FOLDER "${FOLDER_EXTERNAL}/CTRE")

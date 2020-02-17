set(MGPS_SOURCES
    api.hh
    export.hh
    library.cc
    library.hh
    media_file.hh
    loader/dynamic_plugin.cc
    loader/dynamic_plugin.hh
    loader/host.cc
    loader/host.hh
    loader/loader.hh
    loader/parser/elf_parser.hh
    loader/parser/elf_parser.cc
    loader/parser/mach_parser.hh
    loader/parser/mach_parser.cc
    loader/parser/pecoff_parser.hh
    loader/parser/pecoff_parser.cc
    loader/plugin_info.cc
    loader/plugin_info.hh
    loader/plugin_interface.cc
    loader/plugin_interface.hh
    track/point.cc
    track/polyline.cc
    track/trace.cc
    trip.cc
    trip.hh
    version.cc
)

if (UNIX)
    list(APPEND MGPS_SOURCES
        loader/loader.posix.cc
    )
else()
    list(APPEND MGPS_SOURCES
        loader/loader.win32.cc
    )
endif()

add_library(mgps SHARED ${MGPS_SOURCES})

target_include_directories(mgps PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(mgps PUBLIC fs-polyfill mgps-common PRIVATE mgps-plugin)
target_compile_definitions(mgps PRIVATE MGPS_COMPILED=1)
if (NOT MSVC)
    target_compile_options(mgps PRIVATE -fvisibility=hidden)
endif()

if (UNIX)
    target_link_libraries(mgps PUBLIC dl)
endif()

dir_filters(MGPS_SOURCES)
set_target_properties(mgps PROPERTIES
    FOLDER ${FOLDER_CORE}
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS mgps 
    ARCHIVE COMPONENT dev
    OBJECTS COMPONENT dev
    LIBRARY
        COMPONENT library
        NAMELINK_COMPONENT dev
    RUNTIME COMPONENT library)
install(DIRECTORY ./
    COMPONENT dev
    DESTINATION include/mgps
    FILES_MATCHING PATTERN "*.hh")

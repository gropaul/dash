cmake_minimum_required(VERSION 3.5)
project(dash)

# CMake example
string(REGEX MATCH "v([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ ${DUCKDB_VERSION})
set(DUCKDB_MAJOR_VERSION "${CMAKE_MATCH_1}")
set(DUCKDB_MINOR_VERSION "${CMAKE_MATCH_2}")
set(DUCKDB_PATCH_VERSION "${CMAKE_MATCH_3}")
add_compile_definitions(
    DUCKDB_MAJOR_VERSION=${DUCKDB_MAJOR_VERSION}
    DUCKDB_MINOR_VERSION=${DUCKDB_MINOR_VERSION}
    DUCKDB_PATCH_VERSION=${DUCKDB_PATCH_VERSION}
)

# Set extension name here
set(TARGET_NAME dash)

if(MINGW)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()

find_package(OpenSSL REQUIRED)

include_directories(src/include duckdb/third_party/httplib)

set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

set(EXTENSION_SOURCES
        src/dash_extension.cpp
        src/gen/files.cpp
        src/temp_file.cpp
)


# 3) Build the extensions
build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

# 4) Link OpenSSL
target_link_libraries(${EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${LOADABLE_EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto)

# 5) Make sure the extension depends on the script target
#add_dependencies(${EXTENSION_NAME} run_my_python_script)
#add_dependencies(${LOADABLE_EXTENSION_NAME} run_my_python_script)

install(
        TARGETS ${EXTENSION_NAME}
        EXPORT "${DUCKDB_EXPORT_SET}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
)

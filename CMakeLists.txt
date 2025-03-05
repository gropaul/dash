cmake_minimum_required(VERSION 3.5)
project(duck_explorer)

# 1) Find Python
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Set extension name here
set(TARGET_NAME duck_explorer)

if(MINGW)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()

find_package(OpenSSL REQUIRED)

include_directories(src/include duckdb/third_party/httplib)

set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

set(EXTENSION_SOURCES
        src/duck_explorer_extension.cpp
        src/gen/files.cpp
        src/serializer/result_serializer.cpp
        src/temp_file.cpp
)

# 2) Add a custom target that runs your Python script
# The `ALL` ensures the script will run whenever you type `make`
# (or build the default target).
add_custom_target(
        run_my_python_script ALL
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_ui.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}  # or wherever you need to run
        COMMENT "Running Python script to build UI"
)

# 3) Build the extensions
build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

# 4) Link OpenSSL
target_link_libraries(${EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${LOADABLE_EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto)

# 5) Make sure the extension depends on the script target
add_dependencies(${EXTENSION_NAME} run_my_python_script)
add_dependencies(${LOADABLE_EXTENSION_NAME} run_my_python_script)

install(
        TARGETS ${EXTENSION_NAME}
        EXPORT "${DUCKDB_EXPORT_SET}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
)

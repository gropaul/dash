PROJ_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Configuration of extension
EXT_NAME=duck_explorer
EXT_CONFIG=${PROJ_DIR}extension_config.cmake

# Include the Makefile from extension-ci-tools
include extension-ci-tools/makefiles/duckdb_extension.Makefile

release:
	pwd
	ls -al
	ls -al src
	ls -al src/gen
	cat
	mkdir -p build/release
	cmake $(GENERATOR) $(BUILD_FLAGS) $(EXT_RELEASE_FLAGS) -DCMAKE_BUILD_TYPE=Release -S $(DUCKDB_SRCDIR) -B build/release
	cmake --build build/release --config Release

debug:
	pwd
	ls -al
	ls -al src
	ls -al src/gen
	mkdir -p  build/debug
	cmake $(GENERATOR) $(BUILD_FLAGS) $(EXT_DEBUG_FLAGS) -DCMAKE_BUILD_TYPE=Debug -S $(DUCKDB_SRCDIR) -B build/debug
	cmake --build build/debug --config Debug

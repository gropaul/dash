# This file is included by DuckDB's build system. It specifies which extension to load

# Extension from this repo
duckdb_extension_load(dash
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}
    LOAD_TESTS
)
# DuckDB 2.0 links only listed extensions into the test binary; dash_http exists only in the static build.
if(COMMAND duckdb_extension_statically_link)
    duckdb_extension_statically_link(dash)
endif()

# The /api/query endpoint serializes results with to_json, so the tests need json.
duckdb_extension_load(json)
# Claude Code Instructions

## Building

```bash
# Debug build
make debug

# Release build
make release
```

## Testing

Run all extension SQL tests:
```bash
./build/debug/test/unittest --test-dir . "test/*"
```
The Catch tag of a test file is its parent directory, so `"[sql]"` matches only
`test/sql/*.test` and skips `test/sql/http/`.

Run a specific test file:
```bash
./build/debug/test/unittest --test-dir . "test/sql/query_result.test"
```

## Project Structure

- `src/` — C++ extension source code
- `src/include/` — Header files (including `query_result_table_function.hpp`)
- `src/dash_extension.cpp` — Extension entry point and function registration
- `test/sql/` — SQL-based test files (DuckDB `.test` format)
- `test/sql/http/` — tests that drive the HTTP server with the `dash_http` table function,
  which the static extension registers when `DASH_BUILD_TEST_HTTP_CLIENT` is on (the default)
- `duckdb/` — DuckDB submodule
- `extension-ci-tools/` — CI tooling and Makefile includes
#pragma once

#include "duckdb.hpp"

namespace duckdb {
#define DUCKDB_VERSION_ENCODE(major, minor, patch) ((major) * 10000 + (minor) * 100 + (patch))
#define DUCKDB_CURRENT_VERSION DUCKDB_VERSION_ENCODE(DUCKDB_MAJOR_VERSION, DUCKDB_MINOR_VERSION, DUCKDB_PATCH_VERSION)

class DashExtension : public Extension {
public:
#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(1, 3, 3)
	void Load(ExtensionLoader &db) override;
#else
	void Load(DuckDB &db) override;
#endif
	std::string Name() override;
	std::string Version() const override;
};


} // namespace duckdb

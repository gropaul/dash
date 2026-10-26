#pragma once

#include "duckdb.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {
#define DUCKDB_VERSION_ENCODE(major, minor, patch) ((major) * 10000 + (minor) * 100 + (patch))
#define DUCKDB_CURRENT_VERSION DUCKDB_VERSION_ENCODE(DUCKDB_MAJOR_VERSION, DUCKDB_MINOR_VERSION, DUCKDB_PATCH_VERSION)

// DuckDB 2.0 names columns and named parameters with Identifier instead of string
#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(2, 0, 0)
using column_name_t = Identifier;
#else
using column_name_t = string;
#endif

// DuckDB 2.0 declares optional named parameters as options of a typed "**options" parameter
inline void AddNamedParameter(TableFunction &function, const string &name, LogicalType type) {
#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(2, 0, 0)
	auto &signature = function.GetSignature();
	auto add = [&](TypedKwargs &options) { options.Add(Identifier(name), type); };
	if (signature.GetTypedKwargs()) {
		signature.ExtendTypedKwargs(add);
	} else {
		signature.WithTypedKwargs("options", add);
	}
#else
	function.named_parameters[name] = std::move(type);
#endif
}

// DuckDB 2.0 folds MaterializedQueryResult into QueryResult
#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(2, 0, 0)
using materialized_result_t = QueryResult;
#else
using materialized_result_t = MaterializedQueryResult;
#endif

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

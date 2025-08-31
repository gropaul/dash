#pragma once

#include "fmt/format.h"

#include <string>
namespace duckdb {
inline std::string escape_quotes(const std::string &input) {
	std::string result = input;
	size_t pos = 0;
	while ((pos = result.find('\'', pos)) != std::string::npos) {
		result.replace(pos, 1, "''"); // replace ' with ''
		pos += 2;                     // advance past the replacement
	}
	return result;
}


inline void TryGetQueryColumns(Connection &conn,
						  const std::string &query,
						  std::vector<std::string> &names,
						  std::vector<LogicalType> &types) {
	// sanitize query (remove semicolons)
	std::string clean_query = query;
	clean_query.erase(std::remove(clean_query.begin(), clean_query.end(), ';'),
					  clean_query.end());

	// run DESCRIBE
	const std::string describe_query = duckdb_fmt::format("DESCRIBE ({})", clean_query);
	const auto describe_result = conn.Query(describe_query);

	if (describe_result->HasError()) {
		// if this does not work, we try it with a prepared statement
		const auto prep_statement = conn.Prepare(clean_query);
		if (prep_statement->HasError()) {
			throw Exception(prep_statement->error.Type(), prep_statement->error.RawMessage());
		}
		const auto &prep_names = prep_statement->GetNames();
		const auto &prep_types = prep_statement->GetTypes();

		names.insert(names.end(), prep_names.begin(), prep_names.end());
		types.insert(types.end(), prep_types.begin(), prep_types.end());

		return;
	}

	const idx_t n_types = describe_result->RowCount();

	names.clear();
	types.clear();
	names.reserve(n_types);
	types.reserve(n_types);

	for (idx_t row_idx = 0; row_idx < n_types; row_idx++) {
		std::string name = describe_result->GetValue(0, row_idx).ToString();
		std::string type_string = describe_result->GetValue(1, row_idx).ToString();

		names.push_back(name);
		types.push_back(TransformStringToLogicalType(type_string));
	}
}

} // namespace duckdb
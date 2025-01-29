#pragma once

#include "duckdb/main/materialized_query_result.hpp"
#include "duckdb/main/query_result.hpp"
#include "string.hpp"
#include "yyjson.hpp"

namespace duckdb {
using namespace duckdb_yyjson; // NOLINT(*-build-using-namespace)

class ResultSerializer {
public:
	explicit ResultSerializer(const bool _set_invalid_values_to_null)
	    : set_invalid_values_to_null(_set_invalid_values_to_null) {
		doc = yyjson_mut_doc_new(nullptr);
	}

	virtual ~ResultSerializer() {
		yyjson_mut_doc_free(doc);
	}

	virtual std::string Serialize(MaterializedQueryResult &query_result) = 0;

	static unique_ptr<ResultSerializer> Create(ResponseFormat type, bool _set_invalid_values_to_null = false);

protected:
	std::string YY_ToString() {
		auto data = yyjson_mut_write(doc, 0, nullptr);
		if (!data) {
			throw SerializationException("Could not render yyjson document");
		}
		std::string json_output(data);
		free(data);
		return json_output;
	}

	void SerializeInternal(QueryResult &query_result, yyjson_mut_val *append_root, bool values_as_array);

	void SerializeChunk(const DataChunk &chunk, vector<string> &names, vector<LogicalType> &types,
	                    yyjson_mut_val *append_root, bool values_as_array);

	yyjson_mut_val *SerializeRowAsArray(const DataChunk &chunk, idx_t row_idx, vector<LogicalType> &types);

	yyjson_mut_val *SerializeRowAsObject(const DataChunk &chunk, idx_t row_idx, vector<string> &names,
	                                     vector<LogicalType> &types);

	void SerializeValue(yyjson_mut_val *parent, const Value &value, optional_ptr<string> name, const LogicalType &type);

	yyjson_mut_doc *doc;
	bool set_invalid_values_to_null;
};
} // namespace duckdb

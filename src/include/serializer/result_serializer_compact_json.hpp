#pragma once

#include "result_serializer.hpp"

namespace duckdb {

class ResultSerializerCompactJson final : public ResultSerializer {
	friend class ResultSerializer;

	explicit ResultSerializerCompactJson(const bool _set_invalid_values_to_null = false)
	    : ResultSerializer(_set_invalid_values_to_null) {
		root = yyjson_mut_obj(doc);
		D_ASSERT(root);
		yyjson_mut_doc_set_root(doc, root);
	}

	std::string Serialize(MaterializedQueryResult &query_result) override {
		// Metadata about the query result
		yyjson_mut_val *yy_meta = GetMeta(query_result);
		yyjson_mut_obj_add_val(doc, root, "meta", yy_meta);

		// Actual query data
		yyjson_mut_val *yy_data_array = yyjson_mut_arr(doc);
		SerializeInternal(query_result, yy_data_array, true);
		yyjson_mut_obj_add_val(doc, root, "data", yy_data_array);

		// Query statistics
		yyjson_mut_val *yy_stats = GetStats(query_result);
		yyjson_mut_obj_add_val(doc, root, "statistics", yy_stats);

		return YY_ToString();
	}

	yyjson_mut_val *GetMeta(QueryResult &query_result) {
		auto meta_array = yyjson_mut_arr(doc);
		for (idx_t col = 0; col < query_result.ColumnCount(); ++col) {
			auto column_obj = yyjson_mut_obj(doc);
			yyjson_mut_obj_add_strcpy(doc, column_obj, "name", query_result.ColumnName(col).c_str());
			yyjson_mut_arr_append(meta_array, column_obj);
			std::string tp(query_result.types[col].ToString());
			yyjson_mut_obj_add_strcpy(doc, column_obj, "type", tp.c_str());
		}

		return meta_array;
	}

	yyjson_mut_val *GetStats(const MaterializedQueryResult &query_result) {
		auto stat_obj = yyjson_mut_obj(doc);
		yyjson_mut_obj_add_uint(doc, stat_obj, "rows", query_result.RowCount());
		return stat_obj;
	}

	yyjson_mut_val *root;
};
} // namespace duckdb

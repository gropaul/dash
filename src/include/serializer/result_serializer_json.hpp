#pragma once

#include "result_serializer.hpp"

namespace duckdb {

class ResultSerializerJson final : public ResultSerializer {
	friend class ResultSerializer;

	explicit ResultSerializerJson(const bool _set_invalid_values_to_null = false)
	    : ResultSerializer(_set_invalid_values_to_null) {
		root = yyjson_mut_arr(doc);
		D_ASSERT(root);
		yyjson_mut_doc_set_root(doc, root);
	}

	std::string Serialize(MaterializedQueryResult &query_result) override {
		// Actual query data
		SerializeInternal(query_result, root, false);
		return YY_ToString();
	}

	yyjson_mut_val *root;
};
} // namespace duckdb

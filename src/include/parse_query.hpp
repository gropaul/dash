#pragma once

#include "duckdb/main/client_context.hpp"
#include "yyjson.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

#include <result_serializer_compact_json.hpp>

namespace duckdb {
using namespace duckdb_httplib_openssl;
using namespace duckdb_yyjson;

enum class ResponseFormat {
	COMPACT_JSON,
};

struct ExecutionRequest {
	std::string query;
	ResponseFormat format;

	void Execute(const shared_ptr<DatabaseInstance> &db, Response &res) const {
		D_ASSERT(db);

		Connection con(*db);
		auto result = con.Query(query);
		if (result->HasError()) {
			auto error = result->GetErrorObject();
			error.ConvertErrorToJSON();
			res.status = 400;
			res.set_content(error.Message(), "application/json");
			return;
		}

		ResultSerializerCompactJson serializer;
		auto json = serializer.Serialize(*result);
		res.set_content(json, "application/json");
	}
};

inline ExecutionRequest ParseQuery(const Request &req) {
	// TODO remove all exceptions

	yyjson_read_flag flg =
	    YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_INF_AND_NAN;

	// { query: "SELECT * FROM table" }
	yyjson_doc *doc = yyjson_read(req.body.c_str(), req.body.size(), flg);
	if (!doc) {
		throw IOException("Invalid JSON");
	}

	yyjson_val *obj = yyjson_doc_get_root(doc);
	if (!obj || yyjson_get_type(obj) != YYJSON_TYPE_OBJ) {
		yyjson_doc_free(doc);
		throw IOException("Invalid JSON");
	}

	std::string query = yyjson_get_str(yyjson_obj_get(obj, "query"));

	return {
	    query,
	    ResponseFormat::COMPACT_JSON,
	};
}

} // namespace duckdb

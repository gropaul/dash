#pragma once

#include "auto_cleaner.hpp"
#include "duckdb/main/client_context.hpp"
#include "result_serializer_compact_json.hpp"
#include "yyjson.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

namespace duckdb {
using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)
using namespace duckdb_yyjson;          // NOLINT(*-build-using-namespace)

enum class ResponseFormat {
	INVALID,
	COMPACT_JSON,
};

struct ExecutionRequest {
	const std::string query {};
	const ResponseFormat format = ResponseFormat::INVALID;
	ErrorData parsing_error {};

	ExecutionRequest(const std::string &query, ResponseFormat format) : query(query), format(format) {
	}

	explicit ExecutionRequest(ErrorData error) : parsing_error(std::move(error)) {
	}

	void Execute(const shared_ptr<DatabaseInstance> &db, Response &res) const {
		D_ASSERT(db);

		if (parsing_error.HasError()) {
			return RespondError(parsing_error, res);
		}

		D_ASSERT(format != ResponseFormat::INVALID);
		Connection con(*db);
		auto result = con.Query(query);
		if (result->HasError()) {
			auto error = result->GetErrorObject();
			return RespondError(error, res);
		}

		ResultSerializerCompactJson serializer;
		const auto json = serializer.Serialize(*result);
		res.set_content(json, "application/json");
	}

	static ExecutionRequest ParseQuery(const Request &req) {
		const yyjson_read_flag flags = YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_INF_AND_NAN;
		yyjson_doc *doc = yyjson_read(req.body.c_str(), req.body.size(), flags);
		if (!doc) {
			return ExecutionRequest(ErrorData {ExceptionType::HTTP, "Could not parse JSON body"});
		}

		yyjson_val *obj = yyjson_doc_get_root(doc);
		AutoCleaner cleaner([&] { yyjson_doc_free(doc); });

		if (!obj || yyjson_get_type(obj) != YYJSON_TYPE_OBJ) {
			return ExecutionRequest {ErrorData {ExceptionType::HTTP, "Expected JSON object as root"}};
		}

		yyjson_val *query_obj = yyjson_obj_get(obj, "query");
		if (!query_obj || yyjson_get_type(query_obj) != YYJSON_TYPE_STR) {
			return ExecutionRequest {ErrorData {ExceptionType::HTTP, "Expected 'query' field as string"}};
		}

		std::string query = yyjson_get_str(query_obj);
		if (query.empty()) {
			return ExecutionRequest {ErrorData {ExceptionType::HTTP, "Query is empty"}};
		}

		yyjson_val *format_obj = yyjson_obj_get(obj, "format");
		if (!format_obj || yyjson_get_type(format_obj) != YYJSON_TYPE_STR) {
			return ExecutionRequest {ErrorData {ExceptionType::HTTP, "Expected 'format' field as string"}};
		}

		ResponseFormat format = ResponseFormat::INVALID;
		const std::string format_str = yyjson_get_str(format_obj);
		if (format_str == "compact_json") {
			format = ResponseFormat::COMPACT_JSON;
		} else {
			return ExecutionRequest {ErrorData {ExceptionType::HTTP, "Unknown format: " + format_str}};
		}

		return ExecutionRequest {query, format};
	}

private:
	static void RespondError(ErrorData error, Response &res) {
		error.ConvertErrorToJSON();
		res.status = 400;
		res.set_content(error.Message(), "application/json");
	}
};

} // namespace duckdb

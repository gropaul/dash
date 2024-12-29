#pragma once

#include "auto_cleaner.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/client_data.hpp"
#include "result.hpp"
#include "result_serializer_compact_json.hpp"
#include "temp_file.hpp"
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
	const MultipartFormDataMap &files;

	ExecutionRequest(const std::string &query, const ResponseFormat format, const MultipartFormDataMap &files)
	    : query(query), format(format), files(files) {
	}

	Result<std::nullptr_t> Execute(const shared_ptr<DatabaseInstance> &db, Response &res) const {
		D_ASSERT(db);
		D_ASSERT(format != ResponseFormat::INVALID);

		Connection conn(*db);
		std::vector<unique_ptr<TempFile>> temp_files;
		// Create a temporary table for each file by creating the files in the temporary directory, then creating a
		// temporary table for each file
		for (const auto &file : files) {
			const auto &file_name = !file.second.filename.empty() ? file.second.filename : file.second.name;
			if (file_name == "query_json") {
				continue;
			}
			const auto &file_data = file.second.content;
			temp_files.push_back(make_uniq<TempFile>(file_name, file_data));
		}

		for (const auto &file : temp_files) {
			auto create_file_query = "CREATE TEMP TABLE " + KeywordHelper::WriteQuoted(file->GetName()) + " AS FROM " +
			                         KeywordHelper::WriteQuoted(file->GetPath());
			auto result = conn.Query(create_file_query);
			if (result->HasError()) {
				return result->GetErrorObject();
			}
		}

		auto result = conn.Query(query);
		if (result->HasError()) {
			return result->GetErrorObject();
		}

		ResultSerializerCompactJson serializer;
		const auto json = serializer.Serialize(*result);
		res.set_content(json, "application/json");

		return nullptr;
	}

	static Result<ExecutionRequest> FromRequest(const Request &req, const std::string &api_key) {
		RETURN_IF_ERROR(HasCorrectApiKey(api_key, req));

		auto body = GetRequestBody(req);
		RETURN_IF_ERROR(body);

		return ParseQuery(body->first, body->second);
	}

private:
	static Result<std::nullptr_t> HasCorrectApiKey(const std::string &api_key, const Request &req) {
		if (api_key.empty()) {
			return nullptr;
		}

		const auto &api_key_header = req.get_header_value("X-Api-Key");
		if (api_key_header.empty()) {
			return ErrorData {ExceptionType::HTTP, "Missing 'X-Api-Key' header"};
		}

		if (api_key_header != api_key) {
			return ErrorData {ExceptionType::HTTP, "Invalid API key"};
		}

		return nullptr;
	}

	static Result<std::pair<std::string, const MultipartFormDataMap &>> GetRequestBody(const Request &req) {
		if (req.is_multipart_form_data()) {
			if (!req.has_file("query_json")) {
				return ErrorData {ExceptionType::HTTP, "Missing 'query_json' file"};
			}

			// Make sure that the files does not have multiple values
			for (auto it = req.files.begin(); it != req.files.end();) {
				auto count = req.files.count(it->first);
				if (count > 1) {
					return ErrorData {ExceptionType::HTTP, "Multiple files with name: " + it->first};
				}
				std::advance(it, count);
			}

			return Result<std::pair<std::string, const MultipartFormDataMap &>>(
			    {req.get_file_value("query_json").content, req.files});
		} else {
			return Result<std::pair<std::string, const MultipartFormDataMap &>>({req.body, req.files});
		}
	}

	static Result<ExecutionRequest> ParseQuery(const std::string &request_str, const MultipartFormDataMap &files) {
		constexpr yyjson_read_flag flags = YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_INF_AND_NAN;
		yyjson_doc *doc = yyjson_read(request_str.c_str(), request_str.size(), flags);
		if (!doc) {
			return ErrorData {ExceptionType::HTTP, "Could not parse JSON body"};
		}

		yyjson_val *obj = yyjson_doc_get_root(doc);
		AutoCleaner cleaner([&] { yyjson_doc_free(doc); });

		if (!obj || yyjson_get_type(obj) != YYJSON_TYPE_OBJ) {
			return ErrorData {ExceptionType::HTTP, "Expected JSON object as root"};
		}

		yyjson_val *query_obj = yyjson_obj_get(obj, "query");
		if (!query_obj || yyjson_get_type(query_obj) != YYJSON_TYPE_STR) {
			return ErrorData {ExceptionType::HTTP, "Expected 'query' field as string"};
		}

		std::string query = yyjson_get_str(query_obj);
		if (query.empty()) {
			return ErrorData {ExceptionType::HTTP, "Query is empty"};
		}

		yyjson_val *format_obj = yyjson_obj_get(obj, "format");
		if (!format_obj || yyjson_get_type(format_obj) != YYJSON_TYPE_STR) {
			return ErrorData {ExceptionType::HTTP, "Expected 'format' field as string"};
		}

		ResponseFormat format = ResponseFormat::INVALID;
		const std::string format_str = yyjson_get_str(format_obj);
		if (format_str == "compact_json") {
			format = ResponseFormat::COMPACT_JSON;
		} else {
			return ErrorData {ExceptionType::HTTP, "Unknown format: " + format_str};
		}

		return ExecutionRequest(query, format, files);
	}
};

} // namespace duckdb

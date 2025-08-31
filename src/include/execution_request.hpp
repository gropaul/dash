#pragma once

#include "auto_cleaner.hpp"
#include "duckdb/main/client_data.hpp"
#include "duckdb/main/connection.hpp"
#include "response_format.hpp"
#include "result.hpp"
#include "serializer/result_serializer.hpp"
#include "temp_file.hpp"
#include "yyjson.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"
#include "utils.hpp"
#include "fmt/format.h"

namespace duckdb {
using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)
using namespace duckdb_yyjson;          // NOLINT(*-build-using-namespace)

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
			if (file_name == "query.json") {
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
				return {InternalServerError_500, result->GetErrorObject()};
			}
		}
		printf("Processing %s\n", query.data());
		const string escaped_query = escape_quotes(query);

		const std::string query_template = R"(
		    WITH data AS MATERIALIZED (
		        FROM query_result('{}')
		    ),
		    json_data AS (
		        SELECT to_json(COLUMNS(*))
		        FROM data
		    ),
			json_list AS (
				SELECT ifnull(list([*COLUMNS(*)]), []) as data
				FROM json_data
			),
		    types_data AS (SELECT ANY_VALUE(typeof(COLUMNS(*))) FROM data),
			types_list_data AS (SELECT [(*COLUMNS(*))] as types_with_null, list_filter(types_with_null, x -> x is not null) as types FROM types_data),
			names_data AS (SELECT ANY_VALUE(alias(COLUMNS(*))) FROM data),
			names_list_data AS (SELECT [(*COLUMNS(*))] as names_with_null, list_filter(names_with_null, x -> x is not null) as names FROM names_data),

			combined_data AS (
				SELECT data as rows, list_transform(list_zip(types, names), x -> {{type: x[1], name: x[2]}}) as columns, names
				FROM json_list
				POSITIONAL JOIN types_list_data
				POSITIONAL JOIN names_list_data
			)
			SELECT json_object('rows', rows, 'columns', columns, 'stats', {{ rows: len(rows) }}), names
			FROM combined_data

		)";
		const string json_query = duckdb_fmt::format(query_template, escaped_query);

		auto result = conn.Query(json_query);
		if (result->HasError()) {
			return {BadRequest_400, result->GetErrorObject()};
		}

		const auto json = result->GetValue(0,0);
		res.set_content(json.ToString(), "application/json");

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
			return HttpErrorData {Unauthorized_401, "Missing 'X-Api-Key' header"};
		}

		if (api_key_header != api_key) {
			return HttpErrorData {Unauthorized_401, "Invalid API key"};
		}

		return nullptr;
	}

	static Result<std::pair<std::string, const MultipartFormDataMap &>> GetRequestBody(const Request &req) {
		if (req.is_multipart_form_data()) {
			if (!req.has_file("query.json")) {
				return HttpErrorData {BadRequest_400, "Missing 'query.json' file"};
			}

			// Make sure that the files does not have multiple values
			for (auto it = req.files.begin(); it != req.files.end();) {
				auto count = req.files.count(it->first);
				if (count > 1) {
					return HttpErrorData {BadRequest_400, "Multiple files with name: " + it->first};
				}
				std::advance(it, count);
			}

			return std::make_pair(req.get_file_value("query.json").content, std::ref(req.files));
		} else {
			return std::make_pair(req.body, std::ref(req.files));
		}
	}

	static Result<ExecutionRequest> ParseQuery(const std::string &request_str, const MultipartFormDataMap &files) {
		constexpr yyjson_read_flag flags = YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_INF_AND_NAN;
		yyjson_doc *doc = yyjson_read(request_str.c_str(), request_str.size(), flags);
		if (!doc) {
			return HttpErrorData {BadRequest_400, "Could not parse JSON body"};
		}

		yyjson_val *obj = yyjson_doc_get_root(doc);
		AutoCleaner cleaner([&] { yyjson_doc_free(doc); });

		if (!obj || yyjson_get_type(obj) != YYJSON_TYPE_OBJ) {
			return HttpErrorData {BadRequest_400, "Expected JSON object as root"};
		}

		yyjson_val *query_obj = yyjson_obj_get(obj, "query");
		if (!query_obj || yyjson_get_type(query_obj) != YYJSON_TYPE_STR) {
			return HttpErrorData {BadRequest_400, "Expected 'query' field as string"};
		}

		std::string query = yyjson_get_str(query_obj);
		if (query.empty()) {
			return HttpErrorData {BadRequest_400, "Query is empty"};
		}

		yyjson_val *format_obj = yyjson_obj_get(obj, "format");
		if (!format_obj || yyjson_get_type(format_obj) != YYJSON_TYPE_STR) {
			return HttpErrorData {BadRequest_400, "Expected 'format' field as string"};
		}

		auto format = ResponseFormat::INVALID;
		const std::string format_str = yyjson_get_str(format_obj);
		try {
			format = string_util::FromString<ResponseFormat>(format_str);
		} catch (const std::exception &ex) {
			return HttpErrorData {BadRequest_400, ex.what()};
		}

		return ExecutionRequest(query, format, files);
	}
};

} // namespace duckdb

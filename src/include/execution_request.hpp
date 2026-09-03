#pragma once

#include "auto_cleaner.hpp"
#include "duckdb/main/client_data.hpp"
#include "duckdb/main/connection.hpp"
#include "duckdb/main/relation/materialized_relation.hpp"
#include "response_format.hpp"
#include "result.hpp"
#include "temp_file.hpp"
#include "yyjson.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"
#include "string_util.hpp"
#include "utils.hpp"
#include "fmt/format.h"

namespace duckdb {
using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)
using namespace duckdb_yyjson;          // NOLINT(*-build-using-namespace)

// Compatibility layer for httplib multipart API changes starting from 1.4.5
#if DUCKDB_VERSION_CODE > 10404
using MultipartFiles = FormFiles;
#define HTTPLIB_NEW_MULTIPART_API 1
#else
using MultipartFiles = MultipartFormDataMap;
#define HTTPLIB_NEW_MULTIPART_API 0
#endif

struct ExecutionRequest {
	const std::string query {};
	const ResponseFormat format = ResponseFormat::INVALID;
	const MultipartFiles &files;

	ExecutionRequest(const std::string &query, const ResponseFormat format, const MultipartFiles &files)
	    : query(query), format(format), files(files) {
	}

	Result<std::nullptr_t> Execute(Connection &conn, Response &res) const {
		D_ASSERT(format != ResponseFormat::INVALID);

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
			// OR REPLACE because the session connection outlives the request that created the table.
			auto create_file_query = "CREATE OR REPLACE TEMP TABLE " + KeywordHelper::WriteQuoted(file->GetName()) +
			                         " AS FROM " + KeywordHelper::WriteQuoted(file->GetPath());
			auto result = conn.Query(create_file_query);
			if (result->HasError()) {
				return {InternalServerError_500, result->GetErrorObject()};
			}
		}

		vector<unique_ptr<SQLStatement>> statements;
		try {
			statements = conn.ExtractStatements(query);
		} catch (const std::exception &ex) {
			return {BadRequest_400, ErrorData(ex)};
		}
		if (statements.empty()) {
			return HttpErrorData {BadRequest_400, "No statements found in query"};
		}

		// Everything before the final statement runs directly on the session connection, so USE, SET
		// and TEMP DDL take effect on the connection that binds the final statement and survive into
		// later requests.
		for (idx_t i = 0; i + 1 < statements.size(); i++) {
			auto result = conn.Query(std::move(statements[i]));
			if (result->HasError()) {
				return {BadRequest_400, result->GetErrorObject()};
			}
		}

		if (statements.back()->type == StatementType::SELECT_STATEMENT) {
			// query_result's bind_replace turns this into a SubqueryRef bound on `conn`, which keeps
			// return types coming from the parse tree and leaves the subquery open to the optimizer.
			return RespondWithJson(conn, "query_result('" + EscapeQutes(statements.back()->query) + "')", res);
		}

		// Anything else has to run on the session connection itself, so its effect on the catalog and
		// on session state is the one later requests see.
		auto result = conn.Query(std::move(statements.back()));
		if (result->HasError()) {
			return {BadRequest_400, result->GetErrorObject()};
		}
		return RespondWithResult(conn, std::move(result), res);
	}

	static Result<ExecutionRequest> FromRequest(const Request &req, const std::string &api_key) {
		RETURN_IF_ERROR(HasCorrectApiKey(api_key, req));

		auto body = GetRequestBody(req);
		RETURN_IF_ERROR(body);

		return ParseQuery(body->first, body->second);
	}

private:

	// Serialization stays in SQL: to_json handles arbitrarily nested DuckDB types, which is not
	// something worth reimplementing in C++. `data_source` is any table expression.
	static string JsonQuery(const string &data_source) {
		const std::string query_template = R"(
		    WITH data AS (
		        FROM {}
		    ),
			dash_row_number_ids AS (
			        SELECT range as dash_row_number_id
			        FROM range((SELECT COUNT(*) FROM data))
			),
			json_data AS (
                    SELECT dash_row_number_ids.dash_row_number_id, to_json(COLUMNS(* EXCLUDE (dash_row_number_id)))
			        FROM data
			        POSITIONAL JOIN dash_row_number_ids
			),
			json_list AS (
			        SELECT ifnull(list([*COLUMNS(* EXCLUDE (dash_row_number_id))] ORDER BY dash_row_number_id), []) as data
			        FROM json_data
			),
		    types_data AS (SELECT ANY_VALUE(typeof(COLUMNS(*))) FROM data),
			types_list_data AS (SELECT [(*COLUMNS(*))] as types_with_null, list_filter(types_with_null, lambda x : x is not null) as types FROM types_data),
			names_data AS (SELECT ANY_VALUE(alias(COLUMNS(*))) FROM data),
			names_list_data AS (SELECT [(*COLUMNS(*))] as names_with_null, list_filter(names_with_null, lambda x : x is not null) as names FROM names_data),

			combined_data AS (
				SELECT data as rows, list_transform(list_zip(types, names), lambda x : {{type: x[1], name: x[2]}}) as columns, names
				FROM json_list
				POSITIONAL JOIN types_list_data
				POSITIONAL JOIN names_list_data
			)
			SELECT json_object('rows', rows, 'columns', columns, 'stats', {{ rows: len(rows) }}), names
			FROM combined_data

		)";
		return duckdb_fmt::format(query_template, data_source);
	}

	static Result<std::nullptr_t> RespondWithJson(Connection &conn, const string &data_source, Response &res) {
		auto result = conn.Query(JsonQuery(data_source));
		if (result->HasError()) {
			return {BadRequest_400, result->GetErrorObject()};
		}
		res.set_content(result->GetValue(0, 0).ToString(), "application/json");
		return nullptr;
	}

	// A statement that already ran cannot be re-run inside the JSON wrapper, so its result is bound
	// back into SQL as an in-memory relation and serialized by the same template.
	static Result<std::nullptr_t> RespondWithResult(Connection &conn, unique_ptr<MaterializedQueryResult> result,
	                                                Response &res) {
		// CreateView takes a bare identifier; everything referencing it qualifies the temp schema so
		// a user's USE cannot move the lookup elsewhere.
		static constexpr const char *RESULT_VIEW = "__dash_last_result";
		static constexpr const char *RESULT_VIEW_REF = "temp.main.__dash_last_result";

#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(2, 0, 0)
		auto names = result->GetNames();
#else
		auto names = result->names;
#endif
		auto relation = make_shared_ptr<MaterializedRelation>(conn.context, result->TakeCollection(), names);
		auto json_result = relation->Query(RESULT_VIEW, JsonQuery(RESULT_VIEW_REF));
		auto json_error = json_result->HasError() ? json_result->GetErrorObject() : ErrorData();

		// Release the collection the view pins; it is only needed for the query above.
		conn.Query(string("DROP VIEW IF EXISTS ") + RESULT_VIEW_REF);

		if (json_error.HasError()) {
			return {BadRequest_400, json_error};
		}
		auto materialized = unique_ptr_cast<QueryResult, MaterializedQueryResult>(std::move(json_result));
		res.set_content(materialized->GetValue(0, 0).ToString(), "application/json");
		return nullptr;
	}

	static Result<std::pair<std::string, const MultipartFiles &>> GetRequestBody(const Request &req) {
#if HTTPLIB_NEW_MULTIPART_API
		if (req.is_multipart_form_data()) {
			if (!req.form.has_file("query.json")) {
				return HttpErrorData {BadRequest_400, "Missing 'query.json' file"};
			}

			// Make sure that the files does not have multiple values
			for (auto it = req.form.files.begin(); it != req.form.files.end();) {
				auto count = req.form.files.count(it->first);
				if (count > 1) {
					return HttpErrorData {BadRequest_400, "Multiple files with name: " + it->first};
				}
				std::advance(it, count);
			}

			return std::make_pair(req.form.get_file("query.json").content, std::ref(req.form.files));
		} else {
			static const MultipartFiles empty_files;
			return std::make_pair(req.body, std::ref(empty_files));
		}
#else
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
#endif
	}

	static Result<ExecutionRequest> ParseQuery(const std::string &request_str, const MultipartFiles &files) {
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

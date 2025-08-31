#pragma once

#include "dash_extension.hpp"
#include "duckdb.hpp"
#include "utils.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/main/prepared_statement_data.hpp"
#include "fmt/format.h"

#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

#include <iomanip> // for std::fixed and std::setprecision

namespace duckdb {
struct QueryResultJsonFunctionData final : FunctionData {

	string query;

	explicit QueryResultJsonFunctionData(const string &query_p)
	    : query(query_p) {
	}

	unique_ptr<FunctionData> Copy() const override {
		return make_uniq<QueryResultJsonFunctionData>(this->query);
	}

	bool Equals(const FunctionData &other) const override {
		const auto &otherQR = other.Cast<QueryResultJsonFunctionData>();
		return this->query == otherQR.query;
	}
};

struct QueryResultJsonState final : GlobalTableFunctionState {

	std::atomic_bool run;
	QueryResultJsonState() : run(false) {
	}

	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		return make_uniq<QueryResultJsonState>();
	}
};

const child_list_t<LogicalType> struct_types {make_pair("name", LogicalType::VARCHAR),
                                              make_pair("type", LogicalType::VARCHAR)};
const LogicalType list_type = LogicalType::LIST(LogicalType::STRUCT(struct_types));

static void QueryResultJsonFun(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {

	auto &function_data = data_p.bind_data->Cast<QueryResultJsonFunctionData>();
	auto &state = data_p.global_state->Cast<QueryResultJsonState>();

	if (state.run.exchange(true)) {
		return;
	}

	Connection conn(*context.db);

	const string &query = escape_quotes(function_data.query);

	const std::string query_template = R"(
	    WITH data AS (
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
		types_list_data AS (SELECT [(*COLUMNS(*))] as types FROM types_data),
		names_data AS (SELECT ANY_VALUE(alias(COLUMNS(*))) FROM data),
		names_list_data AS (SELECT [(*COLUMNS(*))] as names FROM names_data)

		SELECT data, list_transform(list_zip(types, names), x -> {{type: x[1], name: x[2]}}) as columns
		FROM json_list
		POSITIONAL JOIN types_list_data
		POSITIONAL JOIN names_list_data;

	)";
	const std::string json_query = duckdb_fmt::format(query_template, query);
	const auto json_result = conn.Query(json_query);

	if (json_result->HasError()) {
		throw Exception(json_result->GetErrorObject().Type(), json_result->GetErrorObject().RawMessage());
	}
	const auto rows = json_result->GetValue(0, 0);
	const auto columns = json_result->GetValue(1, 0);

	output.SetValue(0, 0, rows);
	output.SetValue(1, 0, columns);
	output.SetCardinality(1);
}

static unique_ptr<FunctionData> QueryResultJsonBind(ClientContext &context, TableFunctionBindInput &input,
                                                    vector<LogicalType> &return_types, vector<string> &names) {

	if (input.inputs.size() != 1) {
		throw Exception(ExceptionType::BINDER, "QueryResultJson needs exactly one argument");
	}

	const string query = input.inputs[0].GetValue<string>();

	Connection conn(*context.db);

	return_types.push_back(LogicalType::LIST(LogicalType::LIST(LogicalType::JSON()))); // json result
	names.push_back("rows");

	return_types.push_back(list_type);
	names.push_back("columns");

	auto data = make_uniq<QueryResultJsonFunctionData>(query);
	return std::move(data);
}
} // namespace duckdb
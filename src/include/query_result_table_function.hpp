#pragma once

#include "dash_extension.hpp"
#include "duckdb.hpp"
#include "utils.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/main/prepared_statement_data.hpp"
#include "duckdb/parser/parser.hpp"
#include "duckdb/planner/planner.hpp"

#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

#include <iomanip> // for std::fixed and std::setprecision

namespace duckdb {
struct QueryResultFunctionData final : FunctionData {

	string query;
	explicit QueryResultFunctionData(const string &query_p) : query(query_p) {

	}

	unique_ptr<FunctionData> Copy() const override {
		printf("Copying QueryResultFunctionData");
		throw Exception(ExceptionType::INTERNAL, "Cant copy this");
	}

	bool Equals(const FunctionData &other) const override {
		const auto &otherQR = other.Cast<QueryResultFunctionData>();
		return this->query == otherQR.query;
	}
};

struct QueryResultState final : GlobalTableFunctionState {

	bool has_result;
	unique_ptr<MaterializedQueryResult> result;
	QueryResultState() : has_result(false) {
	}

	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		return make_uniq<QueryResultState>();
	}
};

static void QueryResultFun(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {

	auto &function_data = data_p.bind_data->Cast<QueryResultFunctionData>();
	auto &state = data_p.global_state->Cast<QueryResultState>();
	if (!state.has_result) {
		state.has_result = true;
		Connection conn(*context.db);
		state.result = conn.Query(function_data.query);
	}
	const auto chunk = state.result->Fetch();
	if (chunk) {
		output.Reference(*chunk);
		output.SetCardinality(chunk->size());
	} else {
		output.SetCardinality(0);
	}
}

static unique_ptr<FunctionData> QueryResultBind(ClientContext &context, TableFunctionBindInput &input,
                                                vector<LogicalType> &return_types, vector<string> &names) {

	if (input.inputs.size() != 1) {
		throw Exception(ExceptionType::BINDER, "QueryResult needs exactly one argument");
	}

	Parser parser;
	const string query = input.inputs[0].GetValue<string>();
	parser.ParseQuery(query);
	const auto &statements = parser.statements;
	const auto &last_statement = statements[0];

	const Planner logical_planner(context);
	auto bound_statement = logical_planner.binder->Bind(*last_statement.get());


	// logical_planner.CreatePlan(std::move(statement));

	// Connection conn(*context.db);
	// context.GetTableNames()
	// auto root = conn.ExtractPlan(query);

	// if (result->HasError()) {
	// 	throw Exception(result->GetErrorObject().Type(), result->GetErrorObject().RawMessage());
	// }

	const idx_t column_count = bound_statement.types.size();
	for (int col_idx = 0; col_idx < column_count; col_idx ++) {
		return_types.push_back(bound_statement.types[col_idx]);
		names.push_back(bound_statement.names[col_idx]);
	}

	auto data = make_uniq<QueryResultFunctionData>(query);
	return std::move(data);

}
} // namespace duckdb
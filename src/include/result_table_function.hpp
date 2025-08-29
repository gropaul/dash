#pragma once

#include "dash_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/main/prepared_statement_data.hpp"

#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

#include <iomanip> // for std::fixed and std::setprecision

namespace duckdb {
struct QueryResultFunctionData final : FunctionData {

	string query;
	explicit QueryResultFunctionData(const string query_p) : query(query_p) {
	}

	unique_ptr<FunctionData> Copy() const override {
		return make_uniq<QueryResultFunctionData>(this->query);
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

	const string query = input.inputs[0].GetValue<string>();

	Connection conn(*context.db);
	const auto prepared_statement = conn.Prepare(query);

	if (prepared_statement->HasError()) {
		throw Exception(prepared_statement->error.Type(), prepared_statement->error.RawMessage());
	} else {
		const auto &result_types = prepared_statement->data->types;
		const auto &result_names = prepared_statement->data->names;

		// add the result types and names to the return vectors
		for (idx_t i = 0; i < result_types.size(); i++) {
			return_types.push_back(result_types[i]);
			names.push_back(result_names[i]);
		}
	}


	auto data = make_uniq<QueryResultFunctionData>(query);
	return std::move(data);
}
} // namespace duckdb
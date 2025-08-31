#pragma once

#include "dash_extension.hpp"
#include "duckdb.hpp"
#include "utils.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/main/prepared_statement_data.hpp"

#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

#include <iomanip> // for std::fixed and std::setprecision

namespace duckdb {
struct QueryResultFunctionData final : FunctionData {

	unique_ptr<MaterializedQueryResult> result;
	string query;
	explicit QueryResultFunctionData(const string &query_p, unique_ptr<MaterializedQueryResult> result_p) : result(std::move(result_p)), query(query_p) {

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
	const auto chunk = function_data.result->Fetch();
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

	printf("Query binding is happening\n");
	const string query = input.inputs[0].GetValue<string>();
	Connection conn(*context.db);
	auto result = conn.Query(query);

	if (result->HasError()) {
		throw Exception(result->GetErrorObject().Type(), result->GetErrorObject().RawMessage());
	}

	for (int col_idx = 0; col_idx < result->ColumnCount(); col_idx ++) {
		return_types.push_back(result->types[col_idx]);
		names.push_back(result->names[col_idx]);
	}

	auto data = make_uniq<QueryResultFunctionData>(query, std::move(result));
	return std::move(data);

}
} // namespace duckdb
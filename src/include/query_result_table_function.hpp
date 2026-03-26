#pragma once

#include "dash_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/main/prepared_statement_data.hpp"

namespace duckdb {
struct QueryResultFunctionData final : FunctionData {

	unique_ptr<MaterializedQueryResult> result;
	string query;
	explicit QueryResultFunctionData(const string &query_p, unique_ptr<MaterializedQueryResult> result_p) : result(std::move(result_p)), query(query_p) {

	}

	unique_ptr<FunctionData> Copy() const override {
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

	const string query = input.inputs[0].GetValue<string>();

	Connection conn(*context.db);

	// Extract all statements and execute them, returning only the last result
	auto statements = conn.ExtractStatements(query);
	if (statements.empty()) {
		throw Exception(ExceptionType::PARSER, "No statements found in query");
	}

	unique_ptr<MaterializedQueryResult> result;
	for (idx_t i = 0; i < statements.size(); i++) {
		// Skip SELECT statements that aren't the last statement (they have no side effects)
		if (statements[i]->type == StatementType::SELECT_STATEMENT && i != statements.size() - 1) {
			continue;
		}

		auto pending = conn.PendingQuery(std::move(statements[i]));
		PendingExecutionResult status;
		do {
			status = pending->ExecuteTask();
			if (context.interrupted) {
				conn.Interrupt();
				throw Exception(ExceptionType::INTERRUPT, "Query interrupted");
			}
		} while (!PendingQueryResult::IsResultReady(status) &&
		         status != PendingExecutionResult::EXECUTION_ERROR);
		auto query_result = pending->Execute();
		if (query_result->HasError()) {
			throw Exception(query_result->GetErrorObject().Type(), query_result->GetErrorObject().RawMessage());
		}
		result = unique_ptr_cast<QueryResult, MaterializedQueryResult>(std::move(query_result));
	}

	for (int col_idx = 0; col_idx < result->ColumnCount(); col_idx ++) {
		return_types.push_back(result->types[col_idx]);
		names.push_back(result->names[col_idx]);
	}

	auto data = make_uniq<QueryResultFunctionData>(query, std::move(result));
	return std::move(data);

}
} // namespace duckdb
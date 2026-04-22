#pragma once

#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/main/prepared_statement_data.hpp"
#include "duckdb/parser/parser.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"
#include "duckdb/parser/statement/select_statement.hpp"

namespace duckdb {

// Returns true if a statement type does not modify the catalog or data, meaning
// it's safe to execute on a separate connection without affecting SubqueryRef
// binding on the main transaction (which can't see DDL/DML from other transactions
// due to MVCC snapshot isolation).
static bool IsReadOnlyStatementType(StatementType type) {
	switch (type) {
	case StatementType::SELECT_STATEMENT:
	case StatementType::EXPLAIN_STATEMENT:
	case StatementType::PRAGMA_STATEMENT:
	case StatementType::SET_STATEMENT:
	case StatementType::VARIABLE_SET_STATEMENT:
	case StatementType::CALL_STATEMENT:
	case StatementType::LOAD_STATEMENT:
	case StatementType::RELATION_STATEMENT:
		return true;
	default:
		return false;
	}
}

// Check whether all preceding statements are read-only (settings, pragmas, selects).
// If so, the SubqueryRef optimization is safe because the last SELECT doesn't depend
// on catalog/data changes from the preceding statements.
static bool PrecedingStatementsAreReadOnly(ClientContext &context, const vector<unique_ptr<SQLStatement>> &parsed) {
	for (idx_t i = 0; i < parsed.size() - 1; i++) {
		if (!IsReadOnlyStatementType(parsed[i]->type)) {
			return false;
		}
	}
	return true;
}


static unique_ptr<SubqueryRef> ParseSubquery(const string &query, const ParserOptions &options, const string &err_msg) {
	Parser parser(options);
	parser.ParseQuery(query);
	if (parser.statements.size() != 1) {
		throw ParserException(err_msg);
	}

	auto &stmt = parser.statements[0];

	if (stmt->type == StatementType::SELECT_STATEMENT) {
		// Regular SELECT statement
		auto select_stmt = unique_ptr_cast<SQLStatement, SelectStatement>(std::move(stmt));
		return duckdb::make_uniq<SubqueryRef>(std::move(select_stmt));
	} else if (stmt->type == StatementType::MULTI_STATEMENT) {
		// MultiStatement (e.g., from PIVOT statements that create enum types)
		throw ParserException(
			"PIVOT statements without explicit IN clauses are not supported in query() function. "
			"Please specify the pivot values explicitly, e.g.: PIVOT ... ON col IN (val1, val2, ...)");
	} else {
		throw ParserException(err_msg);
	}
}

// Execute all but the last statement in `statements` on `conn`, with interrupt checking.
static void ExecutePrecedingStatements(Connection &conn, ClientContext &context,
                                       const vector<unique_ptr<SQLStatement>> &statements) {
	for (idx_t i = 0; i < statements.size() - 1; i++) {
		auto pending = conn.PendingQuery(statements[i]->query);
		PendingExecutionResult status;
		do {
			status = pending->ExecuteTask();
			if (context.interrupted) {
				conn.Interrupt();
				throw Exception(ExceptionType::INTERRUPT, "Query interrupted");
			}
		} while (!PendingQueryResult::IsResultReady(status) &&
		         status != PendingExecutionResult::EXECUTION_ERROR);
		auto result = pending->Execute();
		if (result->HasError()) {
			throw Exception(result->GetErrorObject().Type(), result->GetErrorObject().RawMessage());
		}
	}
}

// bind_replace: return a SubqueryRef when the last statement is a SELECT and
// preceding statements (if any) are read-only. This lets the optimizer work on
// the query (filter pushdown, projection pruning, joins, LIMIT, etc.).
//
// When preceding statements modify the catalog or data (CREATE, INSERT, etc.),
// we fall through to bind which executes everything on a separate connection.
// TODO: ideally we'd execute preceding DDL on the same transaction so the SubqueryRef
// path works for all multi-statement queries. Blocked because bind_replace runs with
// the ClientContext mutex held (private), so we can't call PendingQueryInternal on the
// same context. A possible future approach: interrupt the current query, execute the
// DDL, then re-plan — but DuckDB has no re-plan/retry mechanism in the binder today.
static unique_ptr<TableRef> QueryResultBindReplace(ClientContext &context, TableFunctionBindInput &input) {
	if (input.inputs.size() != 1) {
		throw Exception(ExceptionType::BINDER, "query_result needs exactly one argument");
	}

	const string query = input.inputs[0].GetValue<string>();

	Parser parser(context.GetParserOptions());
	parser.ParseQuery(query);

	if (parser.statements.empty()) {
		throw Exception(ExceptionType::PARSER, "No statements found in query");
	}

	auto &last = parser.statements.back();
	if (last->type == StatementType::SELECT_STATEMENT && PrecedingStatementsAreReadOnly(context, parser.statements)) {
		// Safe to execute preceding read-only statements on a separate connection
		// and return the last SELECT as a SubqueryRef
		if (parser.statements.size() > 1) {
			Connection conn(*context.db);
			ExecutePrecedingStatements(conn, context, parser.statements);
		}
		auto subquery_ref = ParseSubquery(query, context.GetParserOptions(), "Expected a single SELECT statement");
		return std::move(subquery_ref);
	}

	// Preceding statements modify catalog/data, or last is not SELECT — fall through
	return nullptr;
}

// Fallback bind + function for non-SELECT last statements.
struct QueryResultFunctionData final : FunctionData {

	unique_ptr<MaterializedQueryResult> result;
	string query;
	explicit QueryResultFunctionData(const string &query_p, unique_ptr<MaterializedQueryResult> result_p)
	    : result(std::move(result_p)), query(query_p) {
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
		throw Exception(ExceptionType::BINDER, "query_result needs exactly one argument");
	}

	const string query = input.inputs[0].GetValue<string>();

	Connection conn(*context.db);

	// Execute all statements on this connection. For multi-statement queries,
	// bind_replace returned nullptr so nothing has been executed yet.
	// For single non-SELECT statements, bind_replace also returned nullptr.
	auto statements = conn.ExtractStatements(query);
	if (statements.empty()) {
		throw Exception(ExceptionType::PARSER, "No statements found in query");
	}

	// Execute preceding statements
	ExecutePrecedingStatements(conn, context, statements);

	// Execute the last statement
	auto pending = conn.PendingQuery(std::move(statements.back()));
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
	auto result = unique_ptr_cast<QueryResult, MaterializedQueryResult>(std::move(query_result));

	for (int col_idx = 0; col_idx < result->ColumnCount(); col_idx++) {
		return_types.push_back(result->types[col_idx]);
		names.push_back(result->names[col_idx]);
	}

	auto data = make_uniq<QueryResultFunctionData>(query, std::move(result));
	return std::move(data);
}

} // namespace duckdb

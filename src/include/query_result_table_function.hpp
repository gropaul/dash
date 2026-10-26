#pragma once

#include "dash_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/catalog/catalog_search_path.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/main/client_data.hpp"
#include "duckdb/main/prepared_statement_data.hpp"
#include "duckdb/parser/parser.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"
#include "duckdb/parser/statement/select_statement.hpp"

namespace duckdb {

static bool ClientContextIsInterrupted(ClientContext &context) {
#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(1, 5, 3)
	return context.IsInterrupted();
#else
	return context.interrupted;
#endif
}

// Returns true if a statement type does not modify the catalog or data, meaning
// it's safe to execute on a separate connection without affecting SubqueryRef
// binding on the main transaction (which can't see DDL/DML from other transactions
// due to MVCC snapshot isolation). Session changes such as SET or USE are copied back.
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

// Check whether all preceding statements can run on a helper connection without changing what
// the last SELECT sees. If so, the SubqueryRef optimization is safe.
static bool PrecedingStatementsAreReadOnly(const vector<unique_ptr<SQLStatement>> &parsed) {
	for (idx_t i = 0; i < parsed.size() - 1; i++) {
		if (!IsReadOnlyStatementType(parsed[i]->type)) {
			return false;
		}
	}
	return true;
}

// Give `target` the session state of `source`: settings, variables and search path.
static void CopySessionState(ClientContext &source, ClientContext &target) {
	target.config = source.config;
	const auto &set_paths = ClientData::Get(source).catalog_search_path->GetSetPaths();
	auto &target_path = *ClientData::Get(target).catalog_search_path;
	if (set_paths.empty()) {
		target_path.Reset();
	} else {
		target_path.Set(set_paths, CatalogSetPathType::SET_DIRECTLY);
	}
}

static Parser MakeParser(ClientContext &context) {
#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(2, 0, 0)
	return Parser(context);
#else
	return Parser(context.GetParserOptions());
#endif
}

// Run `statement` to completion on `conn`, interrupting it when `context` is interrupted.
static unique_ptr<materialized_result_t> ExecuteInterruptibly(Connection &conn, ClientContext &context,
                                                             unique_ptr<SQLStatement> statement) {
#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(2, 0, 0)
	auto result = conn.Submit(std::move(statement));
	if (!result->HasError()) {
		result->Materialize();
		QueryResultState status;
		do {
			status = result->ExecuteTask();
			if (ClientContextIsInterrupted(context)) {
				conn.Interrupt();
				throw Exception(ExceptionType::INTERRUPT, "Query interrupted");
			}
		} while (!IsTerminal(status));
	}
	if (result->HasError()) {
		throw Exception(result->GetErrorObject().Type(), result->GetErrorObject().RawMessage());
	}
	return result;
#else
	auto pending = conn.PendingQuery(std::move(statement));
	PendingExecutionResult status;
	do {
		status = pending->ExecuteTask();
		if (ClientContextIsInterrupted(context)) {
			conn.Interrupt();
			throw Exception(ExceptionType::INTERRUPT, "Query interrupted");
		}
	} while (!PendingQueryResult::IsResultReady(status) && status != PendingExecutionResult::EXECUTION_ERROR);
	auto query_result = pending->Execute();
	if (query_result->HasError()) {
		throw Exception(query_result->GetErrorObject().Type(), query_result->GetErrorObject().RawMessage());
	}
	return unique_ptr_cast<QueryResult, MaterializedQueryResult>(std::move(query_result));
#endif
}

// Execute all but the last statement in `statements` on `conn`, with interrupt checking.
static void ExecutePrecedingStatements(Connection &conn, ClientContext &context,
                                       const vector<unique_ptr<SQLStatement>> &statements) {
	for (idx_t i = 0; i < statements.size() - 1; i++) {
		ExecuteInterruptibly(conn, context, statements[i]->Copy());
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

	auto parser = MakeParser(context);
	parser.ParseQuery(query);

	if (parser.statements.empty()) {
		throw Exception(ExceptionType::PARSER, "No statements found in query");
	}

	auto &last = parser.statements.back();
	if (last->type == StatementType::SELECT_STATEMENT && PrecedingStatementsAreReadOnly(parser.statements)) {
		if (parser.statements.size() > 1) {
			// The helper connection starts from the caller's session and hands it back, so a SET
			// or USE among the preceding statements applies to the last SELECT.
			Connection conn(*context.db);
			CopySessionState(context, *conn.context);
			ExecutePrecedingStatements(conn, context, parser.statements);
			CopySessionState(*conn.context, context);
		}
		auto select_stmt = unique_ptr_cast<SQLStatement, SelectStatement>(std::move(last));
		return make_uniq<SubqueryRef>(std::move(select_stmt));
	}

	// Preceding statements modify catalog/data, or last is not SELECT — fall through
	return nullptr;
}

// Fallback bind + function for non-SELECT last statements.
struct QueryResultFunctionData final : FunctionData {

	unique_ptr<materialized_result_t> result;
	string query;
	explicit QueryResultFunctionData(const string &query_p, unique_ptr<materialized_result_t> result_p)
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

struct QueryResultGlobalState final : GlobalTableFunctionState {

	bool has_result;
	unique_ptr<materialized_result_t> result;
	QueryResultGlobalState() : has_result(false) {
	}

	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		return make_uniq<QueryResultGlobalState>();
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
                                                vector<LogicalType> &return_types, vector<column_name_t> &names) {

	if (input.inputs.size() != 1) {
		throw Exception(ExceptionType::BINDER, "query_result needs exactly one argument");
	}

	const string query = input.inputs[0].GetValue<string>();

	Connection conn(*context.db);
	CopySessionState(context, *conn.context);

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
	auto result = ExecuteInterruptibly(conn, context, std::move(statements.back()));
	CopySessionState(*conn.context, context);

#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(2, 0, 0)
	const auto &result_types = result->GetTypes();
	const auto &result_names = result->GetNames();
#else
	const auto &result_types = result->types;
	const auto &result_names = result->names;
#endif
	for (idx_t col_idx = 0; col_idx < result->ColumnCount(); col_idx++) {
		return_types.push_back(result_types[col_idx]);
		names.push_back(result_names[col_idx]);
	}

	auto data = make_uniq<QueryResultFunctionData>(query, std::move(result));
	return std::move(data);
}

} // namespace duckdb

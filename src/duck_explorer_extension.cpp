#define DUCKDB_EXTENSION_MAIN

#include "duck_explorer_extension.hpp"

#include "duckdb.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "http_server.hpp"
#include "table_functions.hpp"

namespace duckdb {

static void LoadInternal(DatabaseInstance &instance) {
	Connection conn(instance);
	conn.BeginTransaction();
	auto &context = *conn.context;
	auto &catalog = Catalog::GetSystemCatalog(context);

	{
		TableFunction tf(std::string("start_duck_explorer"),
		                 {
		                     LogicalType::VARCHAR, // Host
		                     LogicalType::INTEGER  // Port
		                 },
		                 StartHttpServer, BindStartHttpServer, RunOnceGlobalTableFunctionState::Init);
		tf.named_parameters["api_key"] = LogicalType::VARCHAR;
		CreateTableFunctionInfo tf_info(tf);
		catalog.CreateTableFunction(context, &tf_info);
	}

	{

		TableFunction tf(std::string("stop_duck_explorer"), {}, StopHttpServer, BindStopHttpServer,
		                 RunOnceGlobalTableFunctionState::Init);
		CreateTableFunctionInfo tf_info(tf);
		catalog.CreateTableFunction(context, &tf_info);
	}

	conn.Commit();
}

void DuckExplorerExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string DuckExplorerExtension::Name() {
	return "duck_explorer";
}

std::string DuckExplorerExtension::Version() const {
#ifdef EXT_VERSION_DUCK_EXPLORER
	return EXT_VERSION_DUCK_EXPLORER;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void duck_explorer_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::DuckExplorerExtension>();
}

DUCKDB_EXTENSION_API const char *duck_explorer_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif

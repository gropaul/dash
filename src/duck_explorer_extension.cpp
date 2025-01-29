#define DUCKDB_EXTENSION_MAIN

#include "duck_explorer_extension.hpp"

#include "duckdb.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "http_server.hpp"
#include "json_result_collector.hpp"
#include "table_functions.hpp"

namespace duckdb {

static void LoadInternal(DatabaseInstance &instance) {
	Connection conn(instance);
	conn.Query("INSTALL httpfs; LOAD httpfs;");
	conn.Query("INSTALL json; LOAD json;");
	conn.Query("INSTALL hostfs FROM community; LOAD hostfs;");
	conn.BeginTransaction();

	{
		TableFunction tf(std::string("start_duck_explorer"),
		                 {
		                     LogicalType::VARCHAR, // Host
		                     LogicalType::INTEGER  // Port
		                 },
		                 StartHttpServer, BindStartHttpServer, RunOnceGlobalTableFunctionState::Init);
		tf.named_parameters["api_key"] = LogicalType::VARCHAR;
		tf.named_parameters["enable_cors"] = LogicalType::BOOLEAN;
		tf.named_parameters["ui_proxy"] = LogicalType::VARCHAR;
		ExtensionUtil::RegisterFunction(instance, tf);
	}

	{

		TableFunction tf(std::string("stop_duck_explorer"), {}, StopHttpServer, BindStopHttpServer,
		                 RunOnceGlobalTableFunctionState::Init);
		ExtensionUtil::RegisterFunction(instance, tf);
	}

	{
		pragma_query_t to_json = [](ClientContext &context, const FunctionParameters &type) -> string {
			ClientConfig::GetConfig(context).result_collector = [](ClientContext &c, PreparedStatementData &data) {
				return make_uniq<JsonResultCollector>(c, data);
			};
			return type.values[0].ToString();
		};

		auto parquet_key_fun = PragmaFunction::PragmaCall("to_json", to_json, {LogicalType::VARCHAR});
		ExtensionUtil::RegisterFunction(instance, parquet_key_fun);
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

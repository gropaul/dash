#define DUCKDB_EXTENSION_MAIN

#include "duck_explorer_extension.hpp"

#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#ifndef EMSCRIPTEN
#include "http_server.hpp"
#include "table_functions.hpp"
#endif
#include "json_result_collector.hpp"
#include "response_format.hpp"
#include "string_util.hpp"

namespace duckdb {

static void LoadInternal(DatabaseInstance &instance) {
	Connection conn(instance);
	conn.BeginTransaction();
#ifndef EMSCRIPTEN
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
#endif
	{
		pragma_query_t as_json = [](ClientContext &context, const FunctionParameters &type) -> string {
			ResponseFormat serializer_format;
			const auto format_it = type.named_parameters.find("format");
			if (format_it != type.named_parameters.end()) {
				serializer_format = string_util::FromString<ResponseFormat>(format_it->second.ToString());
			} else {
				serializer_format = ResponseFormat::JSON;
			}

			ClientConfig::GetConfig(context).result_collector = [serializer_format](ClientContext &c,
			                                                                        PreparedStatementData &data) {
				ClientConfig::GetConfig(c).result_collector = nullptr;
				return make_uniq<JsonResultCollector>(c, data, serializer_format);
			};
			return type.values[0].ToString();
		};

		auto as_json_fun = PragmaFunction::PragmaCall("as_json", as_json, {LogicalType::VARCHAR});
		as_json_fun.named_parameters["format"] = LogicalType::VARCHAR;
		ExtensionUtil::RegisterFunction(instance, as_json_fun);
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

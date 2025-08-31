#define DUCKDB_EXTENSION_MAIN

#include "include/dash_extension.hpp"

#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#ifndef EMSCRIPTEN
#include "include/http_server.hpp"
#include "include/table_functions.hpp"
#endif
#include "query_result_table_function.hpp"

namespace duckdb {

static void LoadInternal(ExtensionLoader &loader) {
	Connection conn(loader.GetDatabaseInstance());
	conn.BeginTransaction();
#ifndef EMSCRIPTEN
	{
		TableFunction tf(std::string("start_dash"),
		                 {
		                     LogicalType::VARCHAR, // Host
		                     LogicalType::INTEGER  // Port
		                 },
		                 StartHttpServer, BindStartHttpServer, RunOnceGlobalTableFunctionState::Init);
		tf.named_parameters["api_key"] = LogicalType::VARCHAR;
		tf.named_parameters["enable_cors"] = LogicalType::BOOLEAN;
		tf.named_parameters["ui_proxy"] = LogicalType::VARCHAR;
		tf.named_parameters["open_browser"] = LogicalType::BOOLEAN;

		loader.RegisterFunction(tf);
	}

	{

		TableFunction tf(std::string("stop_dash"), {}, StopHttpServer, BindStopHttpServer,
		                 RunOnceGlobalTableFunctionState::Init);
		loader.RegisterFunction(tf);
	}
#endif
	{

		const pragma_query_t PragmaDash = [](ClientContext &context, const FunctionParameters &type) -> string {
			return "CALL start_dash('localhost', 4200, api_key=CAST(CAST(round(random() * 1000000) AS INT) AS String), enable_cors=False, open_browser=True)";
		};

		PragmaFunction dash = PragmaFunction::PragmaCall("dash", PragmaDash, {});
		loader.RegisterFunction(dash);
	}


	TableFunction query_result("query_result", {LogicalType::VARCHAR}, QueryResultFun, QueryResultBind, QueryResultState::Init);
	loader.RegisterFunction(query_result);

	conn.Commit();
}

void DashExtension::Load(ExtensionLoader &loader) {
	LoadInternal(loader);
}
std::string DashExtension::Name() {
	return "quack";
}

std::string DashExtension::Version() const {
#ifdef EXT_VERSION_QUACK
	return EXT_VERSION_QUACK;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

	DUCKDB_CPP_EXTENSION_ENTRY(quack, loader) {
		duckdb::LoadInternal(loader);
	}
}
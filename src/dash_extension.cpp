#define DUCKDB_EXTENSION_MAIN

#include "include/dash_extension.hpp"

#include "duckdb/main/extension_util.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#ifndef EMSCRIPTEN
#include "include/http_server.hpp"
#include "include/table_functions.hpp"
#endif
#include "query_result_json_table_function.hpp"
#include "query_result_table_function.hpp"
#include "include/json_result_collector.hpp"
#include "include/response_format.hpp"
#include "include/string_util.hpp"

namespace duckdb {

inline void QuackScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &name_vector = args.data[0];
	UnaryExecutor::Execute<string_t, string_t>(
		name_vector, result, args.size(),
		[&](string_t name) {
			return StringVector::AddString(result, "Quack "+name.GetString()+" 🐥");
		});
}

static void LoadInternal(DatabaseInstance &instance) {
	Connection conn(instance);
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
		ExtensionUtil::RegisterFunction(instance, tf);
	}

	{

		TableFunction tf(std::string("stop_dash"), {}, StopHttpServer, BindStopHttpServer,
		                 RunOnceGlobalTableFunctionState::Init);
		ExtensionUtil::RegisterFunction(instance, tf);
	}
#endif
	{

		auto quack_scalar_function = ScalarFunction("quack", {LogicalType::VARCHAR}, LogicalType::VARCHAR, QuackScalarFun);
		ExtensionUtil::RegisterFunction(instance, quack_scalar_function);

		pragma_query_t as_json = [](ClientContext &context, const FunctionParameters &type) -> string {
			ResponseFormat serializer_format;
			const auto format_it = type.named_parameters.find("format");
			if (format_it != type.named_parameters.end()) {
				serializer_format = string_util::FromString<ResponseFormat>(format_it->second.ToString());
			} else {
				serializer_format = ResponseFormat::JSON;
			}

			auto& client_config = ClientConfig::GetConfig(context);
			auto previous_result_collector = client_config.result_collector;
			client_config.result_collector = [serializer_format, previous_result_collector] //
			    (ClientContext & c, PreparedStatementData & data) {
				    ClientConfig::GetConfig(c).result_collector = previous_result_collector;
				    return make_uniq<JsonResultCollector>(c, data, serializer_format);
			    };
			return type.values[0].ToString();
		};

		auto as_json_fun = PragmaFunction::PragmaCall("as_json", as_json, {LogicalType::VARCHAR});
		as_json_fun.named_parameters["format"] = LogicalType::VARCHAR;
		ExtensionUtil::RegisterFunction(instance, as_json_fun);

		const pragma_query_t PragmaDash = [](ClientContext &context, const FunctionParameters &type) -> string {
			return "CALL start_dash('localhost', 4200, api_key=CAST(CAST(round(random() * 1000000) AS INT) AS String), enable_cors=False, open_browser=True)";
		};

		PragmaFunction dash = PragmaFunction::PragmaCall("dash", PragmaDash, {});
		ExtensionUtil::RegisterFunction(instance, dash);
	}


	TableFunction query_result("query_result", {LogicalType::VARCHAR}, QueryResultFun, QueryResultBind, QueryResultState::Init);
	ExtensionUtil::RegisterFunction(instance, query_result);

	TableFunction query_result_json("query_result_json", {LogicalType::VARCHAR}, QueryResultJsonFun, QueryResultJsonBind, QueryResultJsonState::Init);
	ExtensionUtil::RegisterFunction(instance, query_result_json);

	conn.Commit();
}

void DashExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string DashExtension::Name() {
	return "dash";
}

std::string DashExtension::Version() const {
#ifdef EXT_VERSION_DASH
	return EXT_VERSION_DASH;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void dash_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::DashExtension>();
}

DUCKDB_EXTENSION_API const char *dash_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif

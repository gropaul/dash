
#define DUCKDB_EXTENSION_MAIN
#define DUCKDB_VERSION_ENCODE(major, minor, patch) ((major) * 10000 + (minor) * 100 + (patch))
#define DUCKDB_CURRENT_VERSION DUCKDB_VERSION_ENCODE(DUCKDB_MAJOR_VERSION, DUCKDB_MINOR_VERSION, DUCKDB_PATCH_VERSION)
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define DUCKDB_VERSION_CODE DUCKDB_VERSION_ENCODE(DUCKDB_MAJOR_VERSION, DUCKDB_MINOR_VERSION, DUCKDB_PATCH_VERSION)

// Manual calculation since pragma can't evaluate arithmetic expressions
#pragma message("DUCKDB version code: " STRINGIFY(DUCKDB_VERSION_CODE))

#include "include/dash_extension.hpp"

#ifndef EMSCRIPTEN
#include "include/http_server.hpp"
#include "include/table_functions.hpp"
#endif
#include "query_result_table_function.hpp"

#if DUCKDB_CURRENT_VERSION < DUCKDB_VERSION_ENCODE(1, 3, 3)
#include "duckdb/main/extension_util.hpp"
#endif




namespace duckdb {

#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(1, 3, 3)

#pragma message("Loading with ExtensionLoader (DuckDB >= 1.4.0)")

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
	}{

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

extern "C" {

	DUCKDB_CPP_EXTENSION_ENTRY(dash, loader) {
		duckdb::LoadInternal(loader);
	}
}


// *** DUCKDB < v1.3.3 ***

#else

#include "duckdb/main/extension_util.hpp"
#pragma message("Loading with Extension Utils (DuckDB < 1.4.0)")


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
	}{

		TableFunction tf(std::string("stop_dash"), {}, StopHttpServer, BindStopHttpServer,
						 RunOnceGlobalTableFunctionState::Init);
		ExtensionUtil::RegisterFunction(instance, tf);
	}
#endif
{

	const pragma_query_t PragmaDash = [](ClientContext &context, const FunctionParameters &type) -> string {
		return "CALL start_dash('localhost', 4200, api_key=CAST(CAST(round(random() * 1000000) AS INT) AS String), enable_cors=False, open_browser=True)";
	};

	PragmaFunction dash = PragmaFunction::PragmaCall("dash", PragmaDash, {});
	ExtensionUtil::RegisterFunction(instance, dash);

}

	TableFunction query_result("query_result", {LogicalType::VARCHAR}, QueryResultFun, QueryResultBind, QueryResultState::Init);
	ExtensionUtil::RegisterFunction(instance, query_result);
	conn.Commit();
}

void DashExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}

extern "C" {

	DUCKDB_EXTENSION_API void dash_init(duckdb::DatabaseInstance &db) {
		duckdb::DuckDB db_wrapper(db);
		db_wrapper.LoadExtension<duckdb::DashExtension>();
	}

	DUCKDB_EXTENSION_API const char *dash_version() {
		return duckdb::DuckDB::LibraryVersion();
	}
}

#endif


#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif

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
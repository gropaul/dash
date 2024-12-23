#pragma once

#include "duckdb/main/client_context.hpp"
#include "files.hpp"
#include "result_serializer_compact_json.hpp"
#include "yyjson.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

namespace duckdb {

using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)
using namespace duckdb_yyjson;          // NOLINT(*-build-using-namespace)

class DuckExplorerHttpServer {
public:
	DuckExplorerHttpServer() {
		server.Post("/query", std::bind(&DuckExplorerHttpServer::ExecuteQuery, this, std::placeholders::_1,
		                                std::placeholders::_2, std::placeholders::_3));
		server.Get(".*",
		           std::bind(&DuckExplorerHttpServer::ServeUi, this, std::placeholders::_1, std::placeholders::_2));
	}
	~DuckExplorerHttpServer() {
		Stop();
	}

	void Start(ClientContext &c, const std::string &host, const int32_t port) {
		if (started.exchange(true)) {
			throw ExecutorException("Server already started");
		}

		Printer::Print("Starting server on " + host + ":" + std::to_string(port));

		db_instance = c.db;
		server_thread = std::thread([&] {
			if (!server.listen(host, port)) {
				throw ExecutorException("Failed to start HTTP server on " + host + ":" + std::to_string(port));
			}
		});
	}

	void Stop() {
		if (!started.exchange(false)) {
			throw ExecutorException("Server not started");
		}

		Printer::Print("Stopping server");
		server.stop();
		server_thread.join();
		db_instance.reset();
	}

private:
	void ExecuteQuery(const Request &req, Response &res, const ContentReader &) {
		auto db = db_instance.lock();
		D_ASSERT(db);

		yyjson_read_flag flg =
		    YYJSON_READ_ALLOW_COMMENTS | YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_INF_AND_NAN;

		// { query: "SELECT * FROM table" }
		yyjson_doc *doc = yyjson_read(req.body.c_str(), req.body.size(), flg);
		if (!doc) {
			res.status = 400;
			res.set_content("Invalid JSON", "text/plain");
			return;
		}

		yyjson_val *obj = yyjson_doc_get_root(doc);
		if (!obj || yyjson_get_type(obj) != YYJSON_TYPE_OBJ) {
			yyjson_doc_free(doc);
			res.status = 400;
			res.set_content("Invalid JSON", "text/plain");
			return;
		}

		std::string query = yyjson_get_str(yyjson_obj_get(obj, "query"));

		Connection con(*db);
		auto result = con.Query(query);
		if (result->HasError()) {
			auto error = result->GetErrorObject();
			error.ConvertErrorToJSON();
			res.status = 400;
			res.set_content(error.Message(), "application/json");
			return;
		}

		ResultSerializerCompactJson serializer;
		auto json = serializer.Serialize(*result);
		res.set_content(json, "application/json");
	}

	void ServeUi(const Request &req, Response &res) {
		auto file = GetFile(req.path);
		if (!file) {
			res.status = 404;
			res.set_content("Not found", "text/plain");
			return;
		}

		Byte *data = file->content.data();
		size_t size = file->content.size();

		res.set_content(reinterpret_cast<char const *>(data), size, file->content_type);
	}

	std::atomic_bool started {false};
	Server server;
	std::thread server_thread;
	weak_ptr<DatabaseInstance> db_instance;
};

DuckExplorerHttpServer server;

inline DuckExplorerHttpServer &GetServer(ClientContext &) {
	return server;
}

} // namespace duckdb

#pragma once

#include "duckdb/main/client_context.hpp"
#include "files.hpp"
#include "parse_query.hpp"
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
		server.Post("/query", [this](const Request &req, Response &res) { ExecuteQuery(req, res); });
		server.Post("/", [this](const Request &req, Response &res) { ExecuteQueryLegacy(req, res); });
		server.Get("/ping", [](const Request &req, Response &res) { res.body = "pong"; });
		server.Get(".*", [this](const Request &req, Response &res) { ServeUi(req, res); });
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
	void ExecuteQuery(const Request &req, Response &res) const {
		ExecutionRequest::ParseQuery(req).Execute(db_instance.lock(), res);
	}

	void ExecuteQueryLegacy(const Request &req, Response &res) const {
		const ExecutionRequest execution {req.body, ResponseFormat::COMPACT_JSON, nullptr};
		execution.Execute(db_instance.lock(), res);
	}

	void ServeUi(const Request &req, Response &res) const {
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

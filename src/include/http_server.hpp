#pragma once

#include "duckdb/main/client_context.hpp"
#include "files.hpp"
#include "parse_query.hpp"
#include "result.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

namespace duckdb {

using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)
using namespace duckdb_yyjson;          // NOLINT(*-build-using-namespace)

class DuckExplorerHttpServer {
public:
	DuckExplorerHttpServer() {
		server.Post("/query", [this](const Request &req, Response &res) { ExecuteQuery(req, res); });
		server.Get("/ping", [](const Request &req, Response &res) { res.body = "pong"; });
		server.Get(".*", [this](const Request &req, Response &res) { ServeUi(req, res); });
	}
	~DuckExplorerHttpServer() {
		Stop();
	}

	void Start(ClientContext &c, const std::string &host, const int32_t port, const std::string &_api_key) {
		if (started.exchange(true)) {
			throw ExecutorException("Server already started");
		}

		Printer::Print("Starting server on " + host + ":" + std::to_string(port));

		db_instance = c.db;
		api_key = _api_key;

		server_thread = std::thread([host, port, this] {
			if (!server.listen(host, port)) {
				Printer::Print("Failed to start HTTP server on " + host + ":" + std::to_string(port));
				Stop(false);
			}
		});
	}

	void Stop(const bool join_thread = true) {
		if (!started.exchange(false)) {
			return;
		}

		Printer::Print("Stopping server");
		server.stop();
		db_instance.reset();
		if (join_thread) {
			server_thread.join();
		}
	}

private:
	void ExecuteQuery(const Request &req, Response &res) const {
		auto execution_request = ExecutionRequest::FromRequest(req, api_key);
		RETURN_IF_ERROR_CB(execution_request, ([&res](const ErrorData &error) { RespondError(error, res); }))
		auto execution_error = execution_request->Execute(db_instance.lock(), res);
		RETURN_IF_ERROR_CB(execution_error, ([&res](const ErrorData &error) { RespondError(error, res); }));
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

	static void RespondError(ErrorData error, Response &res) {
		error.ConvertErrorToJSON();
		res.status = 400;
		res.set_content(error.Message(), "application/json");
	}

	std::atomic_bool started {false};
	Server server;
	std::string api_key {};
	std::thread server_thread;
	weak_ptr<DatabaseInstance> db_instance;
};

DuckExplorerHttpServer server;

inline DuckExplorerHttpServer &GetServer(ClientContext &) {
	return server;
}

} // namespace duckdb

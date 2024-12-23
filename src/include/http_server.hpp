#pragma once

#include "duckdb/main/client_context.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "files.hpp"
#include "httplib.hpp"
#include "yyjson.hpp"

namespace duckdb {

using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)

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

	void Start(const std::string &host, const int32_t port) {
		if (started.exchange(true)) {
			throw ExecutorException("Server already started");
		}

		Printer::Print("Starting server on " + host + ":" + std::to_string(port));

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
	}

private:
	void ExecuteQuery(const Request &req, Response &res, const ContentReader &) {
		res.set_content("Not implemented", "text/plain");
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
};

DuckExplorerHttpServer server;

inline DuckExplorerHttpServer &GetServer(ClientContext &) {
	return server;
}

} // namespace duckdb

#pragma once

#include "duckdb/main/client_context.hpp"

namespace duckdb {

class DuckExplorerHttpServer {
public:
	void Start(std::string host, uint64_t port) {
		if (started.exchange(true)) {
			throw ExecutorException("Server already started");
		}

		Printer::Print("Starting server on " + host + ":" + std::to_string(port));

		// TODO
	};
	void Stop() {
		if (!started.exchange(false)) {
			throw ExecutorException("Server not started");
		}

		Printer::Print("Stopping server");

		// TODO
	};

private:
	std::atomic_bool started {false};
};

DuckExplorerHttpServer server;

inline DuckExplorerHttpServer &GetServer(ClientContext &) {
	return server;
}

} // namespace duckdb

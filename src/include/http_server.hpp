#pragma once

#include "duckdb/main/client_context.hpp"
#include "execution_request.hpp"
#include "files.hpp"
#include "http_error_data.hpp"
#include "result.hpp"
#include "table_functions_bind_data.hpp"
#include "uri.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

#include <duckdb/common/types/blob.hpp>

namespace duckdb {
using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)
using namespace duckdb_yyjson;          // NOLINT(*-build-using-namespace)
#include <string>

static bool tryCommand(const std::string& command, const std::string& argument) {
	// Ensure argument does not contain unsafe characters
	if (argument.find('"') != std::string::npos || argument.find(';') != std::string::npos ||
		argument.find('|') != std::string::npos) {
		return false; // Prevent command injection
	    }

	// Construct the command safely
	const std::string safeCommand = command + " \"" + argument + "\"";
	// system(...) returns -1 on error; return code depends on shell command success
	return (std::system(safeCommand.c_str()) == 0);
}

static void openURL(const std::string& url) {
	// Try xdg-open first
	if (tryCommand("xdg-open", url)) {
		return;
	}
	// Try open (macOS)
	if (tryCommand("open", url)) {
		return;
	}
	// Finally try start (Windows)
	// The 2> redirection won't work on Windows cmd, but won't break either
	tryCommand("start", url + " 2> nul");
}

class DashHttpServer {
public:
	DashHttpServer() {
		server.Post("/query", [this](const Request &req, Response &res) { ExecuteQuery(req, res); });
		server.Get("/ping", [](const Request &, Response &res) { res.body = "pong"; });
		server.Get(".*", [this](const Request &req, Response &res) { ServeUi(req, res); });
		server.Options(".*", [this](const Request &, Response &res) { AddCorsHeaders(res); });
	}
	~DashHttpServer() {
		Stop();
	}

	void XorEncrypt(std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
		D_ASSERT(!key.empty());
		for (size_t i = 0; i < data.size(); i++) {
			data[i] ^= key[i % key.size()];
		}
	}

	std::string XorEncrypt(const std::string& data, const std::string& key) {
		std::vector<uint8_t> data_vec(data.begin(), data.end());
		std::vector<uint8_t> key_vec(key.begin(), key.end());
		XorEncrypt(data_vec, key_vec);
		return std::string(data_vec.begin(), data_vec.end());
	}

	void Start(ClientContext &c, const StartServerFunctionData &data) {
		if (started.exchange(true)) {
			throw ExecutorException("Server already started");
		}

		std::string base_url = "http://" +  data.host + ":" + std::to_string(data.port);
		std::string user_click_url = base_url;
		// say that the client should use the duckdbhttp API
		// without specifying the url parameter, the client will use the same URL as the server
		user_click_url += "/?api=http";
		// add the API key if it is set
		if (!data.api_key.empty()) {
			auto encryption_key = "DuckDB"; // this should not be secure, but just obfuscate the key
			auto encrypted_api_key = XorEncrypt(data.api_key, encryption_key);
			user_click_url += "&k=" + StringUtil::URLEncode(encrypted_api_key);
		}
		if (data.open_browser) {
			openURL(user_click_url);
		}
		Printer::Print("Starting server on " + user_click_url);

		db_instance = c.db;

		api_key = data.api_key;
		enable_cors = data.enable_cors;
		ui_proxy = data.ui_proxy;

		const auto host = data.host;
		const auto port = data.port;
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
		} else {
			server_thread.detach();
		}
	}

private:
	void ExecuteQuery(const Request &req, Response &res) const {
		AddCorsHeaders(res);
		auto execution_request = ExecutionRequest::FromRequest(req, api_key);
		RETURN_IF_ERROR_CB(execution_request, ([&res](const HttpErrorData &error) { RespondError(error, res); }))
		auto execution_error = execution_request->Execute(db_instance.lock(), res);
		RETURN_IF_ERROR_CB(execution_error, ([&res](const HttpErrorData &error) { RespondError(error, res); }));
	}

	void ServeUi(const Request &req, Response &res) const {
		if (ui_proxy.Valid()) {
			ServerFromProxy(req, res);
		} else {
			ServerFromLocal(req, res);
		}
	}

	vector<Byte> Base64Decode(const string &key) const {
		auto result_size = Blob::FromBase64Size(key);
		auto output = duckdb::unique_ptr<unsigned char[]>(new unsigned char[result_size]);
		Blob::FromBase64(key, output.get(), result_size);
		return vector<uint8_t>(output.get(), output.get() + result_size);
	}


	void ServerFromLocal(const Request &req, Response &res) const {
		auto file = GetFile(req.path);
		if (!file) {
			res.status = 404;
			res.set_content("Not found", "text/plain");
			return;
		}

		auto string_content = std::string(file->content, file->content_length);
		auto result_size = Blob::FromBase64Size(string_content);
		vector<Byte> decoded = Base64Decode(file->content);
		unsigned char *decoded_ptr = decoded.data();
		// create a string from decoded bytes
		std::string decoded_str(reinterpret_cast<char *>(decoded_ptr), result_size);

		// get the content as bytes
		res.set_content(decoded_str, file->content_type);
	}

	void ServerFromProxy(const Request &req, Response &res) const {
		Client cli(ui_proxy.HostWithProtocol());
		string path;
		if (StringUtil::StartsWith(req.path, ui_proxy.Path)) {
			path = req.path;
		} else {
			path = ui_proxy.Path + req.path;
		}
		auto response = cli.Get(path);

		if (!response) {
			res.status = 500;
			res.set_content(to_string(response.error()), "text/plain");
			return;
		}

		res.set_content(response->body.c_str(), response->body.size(), response->get_header_value("Content-Type"));
		res.status = response->status;
	}

	static void RespondError(HttpErrorData error, Response &res) {
		error.ConvertErrorToJSON();
		res.status = error.status_code;
		res.set_content(error.Message(), "application/json");
	}

	void AddCorsHeaders(Response &res) const {
		if (!enable_cors) {
			return;
		}
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
		res.set_header("Access-Control-Allow-Headers", "X-Api-Key, Content-Type");
		res.set_header("Access-Control-Allow-Credentials", "true");
		res.set_header("Access-Control-Max-Age", "86400");
	}

	std::atomic_bool started {false};
	Server server;

	string api_key {};
	Uri ui_proxy {};
	bool enable_cors = false;

	std::thread server_thread;
	weak_ptr<DatabaseInstance> db_instance;
};

DashHttpServer server;

inline DashHttpServer &GetServer(ClientContext &) {
	return server;
}

} // namespace duckdb

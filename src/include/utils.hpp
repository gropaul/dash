#pragma once

#include "fmt/format.h"
#include "http_error_data.hpp"
#include "result.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

#include <string>
namespace duckdb {
using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)

inline std::string EscapeQutes(const std::string &input) {
	std::string result = input;
	size_t pos = 0;
	while ((pos = result.find('\'', pos)) != std::string::npos) {
		result.replace(pos, 1, "''"); // replace ' with ''
		pos += 2;                     // advance past the replacement
	}
	return result;
}

inline Result<std::nullptr_t> HasCorrectApiKey(const std::string &api_key, const Request &req) {
	if (api_key.empty()) {
		return nullptr;
	}

	const auto &api_key_header = req.get_header_value("X-Api-Key");
	if (api_key_header.empty()) {
		return HttpErrorData {Unauthorized_401, "Missing 'X-Api-Key' header"};
	}

	if (api_key_header != api_key) {
		return HttpErrorData {Unauthorized_401, "Invalid API key"};
	}

	return nullptr;
}

} // namespace duckdb
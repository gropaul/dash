#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

namespace duckdb {

using namespace duckdb_httplib_openssl;

struct HttpErrorData : ErrorData {
	HttpErrorData() = default;
	HttpErrorData(const StatusCode _status_code, const string &_message)
	    : ErrorData(ExceptionType::HTTP, _message), status_code(_status_code) {
	}
	HttpErrorData(const StatusCode _code, const ErrorData &_error) : ErrorData(_error), status_code(_code) {
	}

	StatusCode status_code = BadRequest_400;
};

} // namespace duckdb

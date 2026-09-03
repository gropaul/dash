#pragma once

#include "dash_extension.hpp"
#include "duckdb/common/file_system.hpp"
#include "execution_request.hpp"
// table_functions.hpp calls GetServer(), which http_server.hpp declares.
#include "http_server.hpp"
#include "table_functions.hpp"
#include "uri.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

namespace duckdb {
using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)

#if HTTPLIB_NEW_MULTIPART_API
using UploadItem = UploadFormData;
using UploadItems = UploadFormDataItems;
#else
using UploadItem = MultipartFormData;
using UploadItems = MultipartFormDataItems;
#endif

//! Bind data for dash_http(), the HTTP client the SQL tests drive the server with.
struct HttpRequestFunctionData final : FunctionData {
	std::string method {};
	//! Scheme, host and port, in the form the httplib client wants them.
	std::string base_url {};
	//! Path including the query string.
	std::string path {};
	std::string body {};
	std::string content_type {};
	Headers headers {};
	//! Paths of files to send as a multipart form, next to the body as 'query.json'.
	vector<std::string> files {};

	unique_ptr<FunctionData> Copy() const override {
		return make_uniq_base<FunctionData, HttpRequestFunctionData>(*this);
	}

	bool Equals(const FunctionData &other_p) const override {
		auto &other = other_p.Cast<HttpRequestFunctionData>();
		return method == other.method && base_url == other.base_url && path == other.path && body == other.body &&
		       content_type == other.content_type && headers == other.headers && files == other.files;
	}
};

inline unique_ptr<FunctionData> BindHttpRequest(ClientContext &, TableFunctionBindInput &input,
                                                vector<LogicalType> &return_types, vector<column_name_t> &names) {
	auto result = make_uniq<HttpRequestFunctionData>();

	result->method = StringUtil::Upper(input.inputs[0].GetValue<string>());
	if (result->method != "GET" && result->method != "POST" && result->method != "OPTIONS") {
		throw BinderException("dash_http supports the GET, POST and OPTIONS methods, got '" + result->method + "'");
	}

	const auto uri = Uri::Parse(input.inputs[1].GetValue<string>());
	uri.AssertValid();
	result->base_url = uri.HostWithProtocol();
	// Uri::Parse keeps the '?' at the front of the query string.
	result->path = (uri.Path.empty() ? "/" : uri.Path) + uri.QueryString;

	result->body = GetOrDefault<string>(input, "body", "");
	result->content_type = GetOrDefault<string>(input, "content_type", "application/json");

	const auto headers = input.named_parameters.find(column_name_t("headers"));
	if (headers != input.named_parameters.end()) {
		for (const auto &entry : MapValue::GetChildren(headers->second)) {
			const auto &pair = StructValue::GetChildren(entry);
			result->headers.emplace(pair[0].ToString(), pair[1].ToString());
		}
	}

	const auto files = input.named_parameters.find(column_name_t("files"));
	if (files != input.named_parameters.end()) {
		for (const auto &entry : ListValue::GetChildren(files->second)) {
			result->files.push_back(entry.ToString());
		}
	}

	return_types = {LogicalType::INTEGER, LogicalType::VARCHAR,
	                LogicalType::MAP(LogicalType::VARCHAR, LogicalType::VARCHAR)};
	names = {column_name_t("status"), column_name_t("body"), column_name_t("headers")};

	return std::move(result);
}

inline UploadItems ReadUploadItems(ClientContext &context, const HttpRequestFunctionData &bind_data) {
	UploadItems items;
	items.push_back(UploadItem {"query.json", bind_data.body, "query.json", "application/json"});

	auto &fs = FileSystem::GetFileSystem(context);
	for (const auto &path : bind_data.files) {
		auto handle = fs.OpenFile(path, FileFlags::FILE_FLAGS_READ);
		const auto size = static_cast<idx_t>(handle->GetFileSize());
		std::string content(size, '\0');
		if (size > 0) {
			handle->Read(&content[0], size);
		}
		const auto name = fs.ExtractName(path);
		items.push_back(UploadItem {name, content, name, "application/octet-stream"});
	}
	return items;
}

//! Response headers as a MAP. httplib keeps a multimap; only the first value of a repeated header
//! survives, because a MAP key has to be unique.
inline Value ResponseHeaders(const Headers &headers) {
	vector<Value> keys;
	vector<Value> values;
	for (auto it = headers.begin(); it != headers.end();) {
		keys.push_back(Value(it->first));
		values.push_back(Value(it->second));
		std::advance(it, headers.count(it->first));
	}
	return Value::MAP(LogicalType::VARCHAR, LogicalType::VARCHAR, std::move(keys), std::move(values));
}

inline void HttpRequest(ClientContext &context, TableFunctionInput &data, DataChunk &output) {
	auto &g_state = data.global_state->Cast<RunOnceGlobalTableFunctionState>();
	if (g_state.has_run.exchange(true)) {
		return;
	}

	const auto &bind_data = data.bind_data->Cast<HttpRequestFunctionData>();

	Client client(bind_data.base_url);
	client.set_connection_timeout(10, 0);
	client.set_read_timeout(120, 0);

	duckdb_httplib_openssl::Result response = [&] {
		if (!bind_data.files.empty()) {
			return client.Post(bind_data.path, bind_data.headers, ReadUploadItems(context, bind_data));
		}
		if (bind_data.method == "GET") {
			return client.Get(bind_data.path, bind_data.headers);
		}
		if (bind_data.method == "OPTIONS") {
			return client.Options(bind_data.path, bind_data.headers);
		}
		return client.Post(bind_data.path, bind_data.headers, bind_data.body, bind_data.content_type);
	}();

	if (!response) {
		throw IOException(bind_data.method + " " + bind_data.base_url + bind_data.path + " failed: " +
		                  to_string(response.error()));
	}

	output.SetCardinality(1);
	output.SetValue(0, 0, Value::INTEGER(response->status));
	output.SetValue(1, 0, Value(response->body));
	output.SetValue(2, 0, ResponseHeaders(response->headers));
}

inline TableFunction GetHttpRequestFunction() {
	TableFunction function("dash_http", {LogicalType::VARCHAR, LogicalType::VARCHAR}, HttpRequest, BindHttpRequest,
	                       RunOnceGlobalTableFunctionState::Init);
	function.named_parameters["body"] = LogicalType::VARCHAR;
	function.named_parameters["content_type"] = LogicalType::VARCHAR;
	function.named_parameters["headers"] = LogicalType::MAP(LogicalType::VARCHAR, LogicalType::VARCHAR);
	function.named_parameters["files"] = LogicalType::LIST(LogicalType::VARCHAR);
	return function;
}

} // namespace duckdb

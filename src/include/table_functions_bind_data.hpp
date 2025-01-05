#pragma once
#include "uri.hpp"

namespace duckdb {
struct StartServerFunctionData final : FunctionData {
	StartServerFunctionData(const string &_host, const int32_t _port, const string &_api_key, const bool _enable_cors,
	                        const Uri &_ui_proxy)
	    : host(_host), port(_port), api_key(_api_key), enable_cors(_enable_cors), ui_proxy(_ui_proxy) {
	}

	unique_ptr<FunctionData> Copy() const override {
		return make_uniq_base<FunctionData, StartServerFunctionData>(host, port, api_key, enable_cors, ui_proxy);
	}

	bool Equals(const FunctionData &other_p) const override {
		auto &other = other_p.Cast<StartServerFunctionData>();
		return host == other.host && port == other.port && api_key == other.api_key &&
		       enable_cors == other.enable_cors && ui_proxy == other.ui_proxy;
	}

	const std::string host;
	const int32_t port;
	const std::string api_key;
	const bool enable_cors;
	const Uri ui_proxy;
};


struct EmptyFunctionData final : FunctionData {
	unique_ptr<FunctionData> Copy() const override {
		return make_uniq_base<FunctionData, EmptyFunctionData>();
	}

	bool Equals(const FunctionData &other_p) const override {
		return true;
	}
};

} // namespace duckdb

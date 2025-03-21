#pragma once
#include "table_functions_bind_data.hpp"

namespace duckdb {
struct RunOnceGlobalTableFunctionState final : GlobalTableFunctionState {
	std::atomic_bool has_run;

	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &, TableFunctionInitInput &) {
		return make_uniq_base<GlobalTableFunctionState, RunOnceGlobalTableFunctionState>();
	}
};

inline void StartHttpServer(ClientContext &context, TableFunctionInput &data, DataChunk &output) {
	auto &g_state = data.global_state->Cast<RunOnceGlobalTableFunctionState>();
	if (g_state.has_run.exchange(true)) {
		return;
	}

	auto &input = data.bind_data->Cast<StartServerFunctionData>();

	GetServer(context).Start(context, input);

	output.SetCardinality(1);
	output.SetValue(0, 0, true);
}

inline unique_ptr<FunctionData> BindStopHttpServer(ClientContext &, TableFunctionBindInput &,
                                                   vector<LogicalType> &return_types, vector<string> &names) {
	return_types.push_back(LogicalType::BOOLEAN);
	names.push_back("success");

	return make_uniq_base<FunctionData, EmptyFunctionData>();
}

inline void StopHttpServer(ClientContext &context, TableFunctionInput &data, DataChunk &output) {
	auto &g_state = data.global_state->Cast<RunOnceGlobalTableFunctionState>();
	if (g_state.has_run.exchange(true)) {
		return;
	}

	GetServer(context).Stop();

	output.SetCardinality(1);
	output.SetValue(0, 0, true);
}

template <typename T>
T GetOrDefault(const TableFunctionBindInput &data, const string &key, T default_value,
               const std::function<void(T &)> &validator = {}) {
	if (data.named_parameters.find(key) != data.named_parameters.end()) {
		auto param = data.named_parameters.at(key).GetValue<T>();
		if (validator) {
			validator(param);
		}
		return param;
	}
	return default_value;
}

inline unique_ptr<FunctionData> BindStartHttpServer(ClientContext &, TableFunctionBindInput &input,
                                                    vector<LogicalType> &return_types, vector<string> &names) {
	auto host = input.inputs[0].GetValue<string>();
	auto port = input.inputs[1].GetValue<uint64_t>();

	return_types.push_back(LogicalType::BOOLEAN);
	names.push_back("success");

	const auto api_key = GetOrDefault<string>(input, "api_key", "", [](const string &value) {
		if (value.empty()) {
			throw BinderException("api_key cannot be an empty string");
		}
	});

	const auto enable_cors = GetOrDefault<bool>(input, "enable_cors", false);
	const auto open_browser = GetOrDefault<bool>(input, "open_browser", false);
	const auto ui_proxy = GetOrDefault<string>(input, "ui_proxy", "", [](const string &value) {
		const auto uri = Uri::Parse(value);
		uri.AssertValid();
		if (uri.Protocol != "http" && uri.Protocol != "https") {
			throw BinderException("ui_proxy must be an HTTP or HTTPS URL");
		}
		if (!uri.QueryString.empty()) {
			throw BinderException("ui_proxy cannot contain a query string");
		}
	});

	return make_uniq_base<FunctionData, StartServerFunctionData>(host, port, api_key, enable_cors, open_browser,
	                                                             Uri::Parse(ui_proxy));
}

} // namespace duckdb

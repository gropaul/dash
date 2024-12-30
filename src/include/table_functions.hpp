#pragma once
namespace duckdb {

struct StartServerFunctionData final : FunctionData {
	StartServerFunctionData(const string &_host, const int32_t _port, const string &_api_key, const bool _enable_cors)
	    : host(_host), port(_port), api_key(_api_key), enable_cors(_enable_cors) {
	}

	unique_ptr<FunctionData> Copy() const override {
		return make_uniq_base<FunctionData, StartServerFunctionData>(host, port, api_key, enable_cors);
	}

	bool Equals(const FunctionData &other_p) const override {
		auto &other = other_p.Cast<StartServerFunctionData>();
		return host == other.host && port == other.port && api_key == other.api_key && enable_cors == other.enable_cors;
	}

	const std::string host;
	const int32_t port;
	const std::string api_key;
	const bool enable_cors;
};

struct EmptyFunctionData final : FunctionData {
	unique_ptr<FunctionData> Copy() const override {
		return make_uniq_base<FunctionData, EmptyFunctionData>();
	}

	bool Equals(const FunctionData &other_p) const override {
		return true;
	}
};

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

	GetServer(context).Start(context, input.host, input.port, input.api_key, input.enable_cors);

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

inline unique_ptr<FunctionData> BindStartHttpServer(ClientContext &, TableFunctionBindInput &input,
                                                    vector<LogicalType> &return_types, vector<string> &names) {
	auto host = input.inputs[0].GetValue<string>();
	auto port = input.inputs[1].GetValue<uint64_t>();

	return_types.push_back(LogicalType::BOOLEAN);
	names.push_back("success");

	string api_key;
	if (input.named_parameters.find("api_key") != input.named_parameters.end()) {
		auto value = input.named_parameters.at("api_key").GetValue<string>();
		if (value.empty()) {
			throw BinderException("api_key cannot be an empty string");
		}
		api_key = value;
	} else {
		api_key = "";
	}

	bool enable_cors = false;
	if (input.named_parameters.find("enable_cors") != input.named_parameters.end()) {
		enable_cors = input.named_parameters.at("enable_cors").GetValue<bool>();
	}

	return make_uniq_base<FunctionData, StartServerFunctionData>(host, port, api_key, enable_cors);
}

} // namespace duckdb

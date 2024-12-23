#pragma once
namespace duckdb {

struct StartServerFunctionData final : FunctionData {
	StartServerFunctionData(const string &_host, const int32_t _port) : host(_host), port(_port) {
	}

	unique_ptr<FunctionData> Copy() const override {
		return make_uniq_base<FunctionData, StartServerFunctionData>(host, port);
	}

	bool Equals(const FunctionData &other_p) const override {
		auto &other = other_p.Cast<StartServerFunctionData>();
		return host == other.host && port == other.port;
	}

	const std::string host;
	const int32_t port;
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

	GetServer(context).Start(context, input.host, input.port);

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

	return make_uniq_base<FunctionData, StartServerFunctionData>(host, port);
}

} // namespace duckdb

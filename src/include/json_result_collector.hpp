#pragma once
#include "duckdb/execution/operator/helper/physical_result_collector.hpp"
#include "duckdb/main/prepared_statement_data.hpp"

namespace duckdb {

struct JsonResultCollectorGlobalState : GlobalSinkState {
	explicit JsonResultCollectorGlobalState(ClientContext &context_p) : context(context_p.shared_from_this()) {
	}
	shared_ptr<ClientContext> context;
	ResultSerializerCompactJson serializer;
};

class JsonResultCollector : public PhysicalResultCollector {
public:
	explicit JsonResultCollector(PreparedStatementData &data) : PhysicalResultCollector(PatchTypesAndNames(data)) {
	}

private:
	SinkResultType Sink(ExecutionContext &context, DataChunk &chunk, OperatorSinkInput &input) const override {
		return SinkResultType::NEED_MORE_INPUT;
	}

	unique_ptr<QueryResult> GetResult(GlobalSinkState &state) override {
		auto &gstate = state.Cast<JsonResultCollectorGlobalState>();

		DataChunk json_buffer;
		json_buffer.Initialize(Allocator::DefaultAllocator(), {LogicalType::VARCHAR}, 2);
		auto context_id_data = FlatVector::GetData<string_t>(json_buffer.data[0]);
		context_id_data[0] = StringVector::AddString(json_buffer.data[0], std::string("hello world"));
		json_buffer.SetCardinality(1);

		auto collection = make_uniq<ColumnDataCollection>(Allocator::DefaultAllocator(), types);
		collection->Append(json_buffer);
		auto client_properties = gstate.context->GetClientProperties();
		return make_uniq<MaterializedQueryResult>(statement_type, properties, names, std::move(collection),
		                                          client_properties);
	}

	unique_ptr<GlobalSinkState> GetGlobalSinkState(ClientContext &context) const override {
		return make_uniq<JsonResultCollectorGlobalState>(context);
	}

private:
	static PreparedStatementData &PatchTypesAndNames(PreparedStatementData &data) {
		data.names = {"json"};
		data.types = {LogicalType::VARCHAR};
		return data;
	}
};
} // namespace duckdb

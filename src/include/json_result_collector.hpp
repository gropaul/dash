#pragma once
#include "duckdb/execution/operator/helper/physical_materialized_collector.hpp"
#include "duckdb/execution/operator/helper/physical_result_collector.hpp"
#include "duckdb/main/prepared_statement_data.hpp"
#include "response_format.hpp"
#include "serializer/result_serializer.hpp"

namespace duckdb {

class JsonResultCollector final : public PhysicalMaterializedCollector {
public:
	explicit JsonResultCollector(ClientContext &context, PreparedStatementData &data, const ResponseFormat format_)
	    : PhysicalMaterializedCollector(data, !PhysicalPlanGenerator::PreserveInsertionOrder(context, *data.plan)),
	      format(format_) {
	}

private:
	unique_ptr<QueryResult> GetResult(GlobalSinkState &state) override {
		unique_ptr<QueryResult> result = PhysicalMaterializedCollector::GetResult(state);
		auto materialized_result = unique_ptr_cast<QueryResult, MaterializedQueryResult>(std::move(result));

		auto serializer = ResultSerializer::Create(format);
		auto serialized = serializer->Serialize(*materialized_result);

		DataChunk json_buffer;
		json_buffer.Initialize(Allocator::DefaultAllocator(), {LogicalType::VARCHAR}, 2);
		auto context_id_data = FlatVector::GetData<string_t>(json_buffer.data[0]);
		context_id_data[0] = StringVector::AddString(json_buffer.data[0], serialized);
		json_buffer.SetCardinality(1);
		auto collection =
		    make_uniq<ColumnDataCollection>(Allocator::DefaultAllocator(), vector<LogicalType> {LogicalType::VARCHAR});
		collection->Append(json_buffer);
		return make_uniq<MaterializedQueryResult>(statement_type, properties, vector<string> {"json"},
		                                          std::move(collection), materialized_result->client_properties);
	}
	const ResponseFormat format;
};
} // namespace duckdb

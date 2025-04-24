#pragma once
#include "duckdb/execution/operator/helper/physical_materialized_collector.hpp"
#include "duckdb/execution/operator/helper/physical_result_collector.hpp"
#include "duckdb/main/prepared_statement_data.hpp"
#include "response_format.hpp"
#include "serializer/result_serializer.hpp"

namespace duckdb {
#define DUCKDB_VERSION_ENCODE(major, minor, patch) ((major) * 10000 + (minor) * 100 + (patch))
#define DUCKDB_CURRENT_VERSION DUCKDB_VERSION_ENCODE(DUCKDB_MAJOR_VERSION, DUCKDB_MINOR_VERSION, DUCKDB_PATCH_VERSION)

#if DUCKDB_CURRENT_VERSION >= DUCKDB_VERSION_ENCODE(1, 3, 0)
static PhysicalOperator* GetPlan(PreparedStatementData &data) {
	std::string version = std::to_string(DUCKDB_CURRENT_VERSION);
	std::cout << version << std::endl;
	return &data.physical_plan->Root();
}

#else
static PhysicalOperator* GetPlan(PreparedStatementData &data) {
	return data.plan.get();
}
#endif


class JsonResultCollector final : public PhysicalMaterializedCollector {
public:
	explicit JsonResultCollector(ClientContext &context, PreparedStatementData &data, const ResponseFormat format_)
	    : PhysicalMaterializedCollector(data, !PhysicalPlanGenerator::PreserveInsertionOrder(context, *GetPlan(data))),
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

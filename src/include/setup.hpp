#pragma once

#include "duckdb.hpp"
#include "duckdb/common/file_system.hpp"

namespace duckdb {

static void AttachDashDatabase(DatabaseInstance &db, Connection &conn) {
	auto &fs = db.GetFileSystem();
	auto home_dir = fs.GetHomeDirectory();
	if (home_dir.empty()) {
		return;
	}
	// DuckDB v1.4.4's FileSystem::JoinPath does not support joining 4+ components in a single call.
	// Build the path incrementally for compatibility across DuckDB versions.
	auto dash_dir = fs.JoinPath(home_dir, ".duckdb");
	dash_dir = fs.JoinPath(dash_dir, "extension_data");
	dash_dir = fs.JoinPath(dash_dir, "dash");
	if (!fs.DirectoryExists(dash_dir)) {
		fs.CreateDirectoriesRecursive(dash_dir);
	}
	auto dash_db_path = fs.JoinPath(dash_dir, "dash.duckdb");
	conn.Query("ATTACH IF NOT EXISTS '" + dash_db_path + "' AS dash");
}

} // namespace duckdb

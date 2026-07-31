#pragma once

#include "duckdb.hpp"
#include "duckdb/common/file_system.hpp"

namespace duckdb {

static string GetDashDirectory(FileSystem &fs) {
	auto home_dir = fs.GetHomeDirectory();
	if (home_dir.empty()) {
		return string();
	}
	// DuckDB v1.4.4's FileSystem::JoinPath does not support joining 4+ components in a single call.
	// Build the path incrementally for compatibility across DuckDB versions.
	auto dash_dir = fs.JoinPath(home_dir, ".duckdb");
	dash_dir = fs.JoinPath(dash_dir, "extension_data");
	dash_dir = fs.JoinPath(dash_dir, "dash");
	return dash_dir;
}

// PRAGMA dash_home — returns the path of GetDashDirectory() as a single-row result.
static string PragmaDashHome(ClientContext &context, const FunctionParameters &parameters) {
	auto &fs = FileSystem::GetFileSystem(context);
	auto dash_dir = GetDashDirectory(fs);
	// Escape single quotes so the path is a valid SQL string literal.
	string escaped;
	for (auto c : dash_dir) {
		if (c == '\'') {
			escaped += "''";
		} else {
			escaped += c;
		}
	}
	return "SELECT '" + escaped + "' AS dash_home";
}

static void AttachDashDatabase(DatabaseInstance &db, Connection &conn) {
	auto &fs = db.GetFileSystem();
	auto dash_dir = GetDashDirectory(fs);
	if (dash_dir.empty()) {
		return;
	}
	if (!fs.DirectoryExists(dash_dir)) {
		fs.CreateDirectoriesRecursive(dash_dir);
	}
	auto dash_db_path = fs.JoinPath(dash_dir, "dash.duckdb");
	conn.Query("ATTACH IF NOT EXISTS '" + dash_db_path + "' AS dash (READ_WRITE)");
}

} // namespace duckdb

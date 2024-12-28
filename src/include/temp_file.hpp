#pragma once

#include "duckdb/common/error_data.hpp"

#include <fstream>

namespace duckdb {

class TempFile {
public:
	TempFile(const std::string &_name, const std::string &_data) : name(_name) {
		assigned_path = GetTempDir() + "/" + std::to_string(::rand()) + "_" + name;

		std::ofstream file(assigned_path, std::ios::binary);
		if (!file.good()) {
			error = ErrorData(ExceptionType::IO, "Could not open file " + assigned_path + " for writing");
			return;
		}

		file << _data;
	}

	TempFile(const TempFile &) = delete;
	TempFile &operator=(const TempFile &) = delete;

	~TempFile() {
		remove(GetPath().c_str());
	}

	std::string GetPath() const {
		return assigned_path;
	}

	std::string GetName() const {
		return name;
	}

	ErrorData GetError() const {
		return error;
	}

	static std::string GetTempDir();

private:
	std::string name;
	std::string assigned_path;
	ErrorData error;
};

} // namespace duckdb

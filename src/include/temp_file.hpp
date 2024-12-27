#pragma once

#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

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

	static std::string GetTempDir() {
#ifdef _WIN32
		char temp_path[MAX_PATH];
		GetTempPathA(MAX_PATH, temp_path);
		return temp_path;
#else
		const char *val = nullptr;
		// ReSharper disable CppUsingResultOfAssignmentAsCondition
		(val = std::getenv("TMPDIR")) ||   //
		    (val = std::getenv("TMP")) ||  //
		    (val = std::getenv("TEMP")) || //
		    (val = std::getenv("TEMPDIR"));
		// ReSharper restore CppUsingResultOfAssignmentAsCondition
#ifdef __ANDROID__
		auto *default_tmp = "/data/local/tmp";
#else
		auto default_tmp = "/tmp";
#endif
		return val ? val : default_tmp;
#endif
	}

private:
	std::string name;
	std::string assigned_path;
	ErrorData error;
};

} // namespace duckdb

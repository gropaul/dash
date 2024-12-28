#include "include/temp_file.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

std::string duckdb::TempFile::GetTempDir() {
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

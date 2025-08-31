#pragma once

#include "fmt/format.h"

#include <string>
namespace duckdb {
inline std::string EscapeQutes(const std::string &input) {
	std::string result = input;
	size_t pos = 0;
	while ((pos = result.find('\'', pos)) != std::string::npos) {
		result.replace(pos, 1, "''"); // replace ' with ''
		pos += 2;                     // advance past the replacement
	}
	return result;
}

} // namespace duckdb
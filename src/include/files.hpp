#pragma once

#include "duckdb/common/optional_ptr.hpp"

#include <string>
#include <vector>

namespace duckdb {

typedef std::string Path;
typedef unsigned char Byte;

struct File {
	const char* content;  // base64 encoded
	size_t content_length;
	std::string content_type;
	std::string path;
};

optional_ptr<File> GetFile(Path path, bool try_resolve_404 = true);

} // namespace duckdb

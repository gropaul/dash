import glob
import os

root_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
target_file = os.path.join(root_dir, "src", "gen", "files.cpp")

CONTENT_TEMPLATE = """
#pragma once
#include "files.hpp"
namespace duckdb {
File %var_name% = {
     // Content
     %content%,    //
     R"__(%content_type%)__", //
     R"__(%path%)__", //
};
}
"""

FILE_TEMPLATE = """
#include "files.hpp"

#include "duckdb/common/optional_ptr.hpp"
#include "duckdb/common/string_util.hpp"

%include%

#include <map>

namespace duckdb { 
std::vector<File> files = {
	%content%
};

optional_ptr<File> GetFile(Path path) {
	if (path.empty() || path == "/") {
		path = "index.html";
	}

	// Append / before and after
	path = "/" + path + "/";
	path = StringUtil::Replace(path, "//", "/");

	auto entry = std::find_if(files.begin(), files.end(), [&path](File &x) { return x.path == path; });
	if (entry == files.end()) {
		return nullptr;
	}

	return *entry;
}

}
"""


def normalize_path(path: str) -> str:
    path = f"/{path}/"
    return path.replace("//", "/")


def get_content_type(file: str) -> str:
    content_types = {
        "html": "text/html",
        "css": "text/css",
        "js": "application/javascript",
        "png": "image/png",
        "jpg": "image/jpeg",
        "ico": "image/x-icon",
        "woff": "font/woff",
        "woff2": "font/woff2",
        "ttf": "font/ttf",
        "svg": "image/svg+xml",
        "json": "application/json",
        "txt": "text/plain",
    }
    extension = file.split(".")[-1]
    return content_types[extension]


def main():
    base_path = os.path.join(root_dir, "explorer-ui", "out")
    target_dir = os.path.dirname(target_file)
    if not os.path.exists(base_path):
        raise Exception(f"Path {base_path} does not exist. Build the UI first.")

    files = glob.glob(os.path.join(base_path, "**", "*"), recursive=True)

    content_list: list[str] = []

    for file in files:
        if not os.path.isfile(file):
            continue
        content = open(
            file,
            "rb",
        ).read()

        # Goal: {0x33, 0x55}
        transformed_content = "{"
        for byte in content:
            transformed_content += f"0x{byte:02x}, "
        transformed_content += "}"

        path = normalize_path(file.replace(base_path, ""))

        path_as_name = path.replace("/", "_").replace(".", "_").replace("-", "_")
        final_path = target_dir + "/" + path_as_name + ".hpp"

        templated = (
            CONTENT_TEMPLATE.replace("%path%", path)
            .replace("%content%", transformed_content)
            .replace("%content_type%", get_content_type(file))
            .replace("%var_name%", path_as_name.upper())
        )

        with open(final_path, "w") as f:
            f.write(templated)
        content_list.append(path_as_name)

    content = (
        FILE_TEMPLATE.replace("%content%", ",".join(content_list).upper())
        .lstrip()
        .replace("%include%", "\n".join([f'#include "{f}.hpp"' for f in content_list]))
    )

    os.makedirs(target_dir, exist_ok=True)
    with open(target_file, "w") as f:
        f.write(content)


if __name__ == "__main__":
    main()

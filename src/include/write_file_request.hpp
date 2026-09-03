#pragma once

#include "auto_cleaner.hpp"
#include "duckdb/common/file_system.hpp"
#include "duckdb/main/database.hpp"
#include "http_error_data.hpp"
#include "result.hpp"
#include "utils.hpp"
#include "yyjson.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

#include <cstdlib>
#include <string>

namespace duckdb {
using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)
using namespace duckdb_yyjson;          // NOLINT(*-build-using-namespace)

//! Everything up to (but excluding) the last path separator. Returns an empty string for a bare
//! file name, and for a file directly under the root - in both cases there is nothing to create.
inline std::string ParentDirectory(const std::string &path) {
	const auto separator_pos = path.find_last_of("/\\");
	if (separator_pos == std::string::npos) {
		return std::string();
	}
	return path.substr(0, separator_pos);
}

struct WriteFileRequest {
	const std::string path {};
	const std::string content {};
	const bool allow_override = false;

	WriteFileRequest(const std::string &path, const std::string &content, const bool allow_override)
	    : path(path), content(content), allow_override(allow_override) {
	}

	Result<std::nullptr_t> Execute(const shared_ptr<DatabaseInstance> &db, Response &res) const {
		D_ASSERT(db);

		auto &fs = db->GetFileSystem();
		// Resolve '~' so callers can address the home directory without knowing where it is.
		const auto target_path = fs.ExpandPath(path);

		bool overwritten = false;
		try {
			if (fs.DirectoryExists(target_path)) {
				return HttpErrorData {BadRequest_400, "Path is an existing directory: " + target_path};
			}

			overwritten = fs.FileExists(target_path);
			if (overwritten && !allow_override) {
				return HttpErrorData {Conflict_409, "File already exists: " + target_path +
				                                        ". Set 'allow_override' to true to overwrite it."};
			}

			const auto parent_directory = ParentDirectory(target_path);
			if (!parent_directory.empty() && !fs.DirectoryExists(parent_directory)) {
				fs.CreateDirectoriesRecursive(parent_directory);
			}

			// FILE_FLAGS_FILE_CREATE_NEW creates the file, and truncates it when it already exists, so an
			// overwrite never leaves a tail of the previous, longer content behind.
			auto handle = fs.OpenFile(target_path, FileFlags::FILE_FLAGS_WRITE |
			                                           FileFlags::FILE_FLAGS_FILE_CREATE_NEW |
			                                           FileLockType::WRITE_LOCK);
			if (!content.empty()) {
				handle->Write(const_cast<char *>(content.data()), content.size()); // NOLINT(*-const-cast)
			}
			handle->Sync();
			handle->Close();
		} catch (const std::exception &ex) {
			return {InternalServerError_500, ErrorData(ex)};
		}

		return Respond(target_path, overwritten, res);
	}

	static Result<WriteFileRequest> FromRequest(const Request &req, const std::string &api_key) {
		RETURN_IF_ERROR(HasCorrectApiKey(api_key, req));

		return Parse(req.body);
	}

private:
	Result<std::nullptr_t> Respond(const std::string &target_path, const bool overwritten, Response &res) const {
		yyjson_mut_doc *doc = yyjson_mut_doc_new(nullptr);
		if (!doc) {
			return HttpErrorData {InternalServerError_500, "Could not allocate the response body"};
		}
		AutoCleaner doc_cleaner([&] { yyjson_mut_doc_free(doc); });

		yyjson_mut_val *root = yyjson_mut_obj(doc);
		yyjson_mut_doc_set_root(doc, root);
		yyjson_mut_obj_add_strcpy(doc, root, "path", target_path.c_str());
		yyjson_mut_obj_add_uint(doc, root, "bytes_written", content.size());
		yyjson_mut_obj_add_bool(doc, root, "overwritten", overwritten);

		char *json = yyjson_mut_write(doc, 0, nullptr);
		if (!json) {
			return HttpErrorData {InternalServerError_500, "Could not serialize the response body"};
		}
		AutoCleaner json_cleaner([&] { free(json); });

		res.status = 200;
		res.set_content(json, "application/json");
		return nullptr;
	}

	static Result<WriteFileRequest> Parse(const std::string &request_str) {
		constexpr yyjson_read_flag flags = YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_INF_AND_NAN;
		yyjson_doc *doc = yyjson_read(request_str.c_str(), request_str.size(), flags);
		if (!doc) {
			return HttpErrorData {BadRequest_400, "Could not parse JSON body"};
		}
		AutoCleaner cleaner([&] { yyjson_doc_free(doc); });

		yyjson_val *obj = yyjson_doc_get_root(doc);
		if (!obj || yyjson_get_type(obj) != YYJSON_TYPE_OBJ) {
			return HttpErrorData {BadRequest_400, "Expected JSON object as root"};
		}

		yyjson_val *path_obj = yyjson_obj_get(obj, "path");
		if (!path_obj || yyjson_get_type(path_obj) != YYJSON_TYPE_STR) {
			return HttpErrorData {BadRequest_400, "Expected 'path' field as string"};
		}
		const std::string path = yyjson_get_str(path_obj);
		if (path.empty()) {
			return HttpErrorData {BadRequest_400, "Path is empty"};
		}

		yyjson_val *content_obj = yyjson_obj_get(obj, "content");
		if (!content_obj || yyjson_get_type(content_obj) != YYJSON_TYPE_STR) {
			return HttpErrorData {BadRequest_400, "Expected 'content' field as string"};
		}
		const std::string content(yyjson_get_str(content_obj), yyjson_get_len(content_obj));

		// Optional, and false when absent: overwriting is opt-in.
		bool allow_override = false;
		yyjson_val *allow_override_obj = yyjson_obj_get(obj, "allow_override");
		if (allow_override_obj) {
			if (yyjson_get_type(allow_override_obj) != YYJSON_TYPE_BOOL) {
				return HttpErrorData {BadRequest_400, "Expected 'allow_override' field as boolean"};
			}
			allow_override = yyjson_get_bool(allow_override_obj);
		}

		return WriteFileRequest(path, content, allow_override);
	}
};

} // namespace duckdb

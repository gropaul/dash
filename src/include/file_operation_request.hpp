#pragma once

#include "auto_cleaner.hpp"
#include "duckdb/common/file_system.hpp"
#include "duckdb/main/database.hpp"
#include "http_error_data.hpp"
#include "result.hpp"
#include "utils.hpp"
#include "write_file_request.hpp"
#include "yyjson.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.hpp"

#include <cstdlib>
#include <string>

namespace duckdb {
using namespace duckdb_httplib_openssl; // NOLINT(*-build-using-namespace)
using namespace duckdb_yyjson;          // NOLINT(*-build-using-namespace)

enum class FileOperation : uint8_t { MOVE, REMOVE, MAKE_DIRECTORY };

//! The file operations next to POST /api/write-file, for clients that keep files on the server:
//! - /api/move-file {"from", "to"}: a file or a directory; the target's parent directories are created.
//! - /api/remove-file {"path"}: a file, or a directory with its contents; a missing path is not an error.
//! - /api/make-directory {"path"}: with its parent directories; an existing one is not an error.
struct FileOperationRequest {
	const FileOperation operation;
	const std::string path {};
	const std::string target {};

	FileOperationRequest(const FileOperation operation, const std::string &path, const std::string &target)
	    : operation(operation), path(path), target(target) {
	}

	Result<std::nullptr_t> Execute(const shared_ptr<DatabaseInstance> &db, Response &res) const {
		D_ASSERT(db);

		auto &fs = db->GetFileSystem();
		const auto source_path = fs.ExpandPath(path);
		bool changed = true;
		try {
			switch (operation) {
			case FileOperation::MOVE: {
				const auto target_path = fs.ExpandPath(target);
				if (!fs.FileExists(source_path) && !fs.DirectoryExists(source_path)) {
					return HttpErrorData {NotFound_404, "No such file or directory: " + source_path};
				}
				const auto parent_directory = ParentDirectory(target_path);
				if (!parent_directory.empty() && !fs.DirectoryExists(parent_directory)) {
					fs.CreateDirectoriesRecursive(parent_directory);
				}
				fs.MoveFile(source_path, target_path);
				break;
			}
			case FileOperation::REMOVE:
				if (fs.DirectoryExists(source_path)) {
					fs.RemoveDirectory(source_path);
				} else {
					changed = fs.TryRemoveFile(source_path);
				}
				break;
			case FileOperation::MAKE_DIRECTORY:
				changed = !fs.DirectoryExists(source_path);
				if (changed) {
					fs.CreateDirectoriesRecursive(source_path);
				}
				break;
			}
		} catch (const std::exception &ex) {
			return {InternalServerError_500, ErrorData(ex)};
		}

		return Respond(source_path, changed, res);
	}

	static Result<FileOperationRequest> FromRequest(const FileOperation operation, const Request &req,
	                                                const std::string &api_key) {
		RETURN_IF_ERROR(HasCorrectApiKey(api_key, req));

		return Parse(operation, req.body);
	}

private:
	Result<std::nullptr_t> Respond(const std::string &source_path, const bool changed, Response &res) const {
		yyjson_mut_doc *doc = yyjson_mut_doc_new(nullptr);
		if (!doc) {
			return HttpErrorData {InternalServerError_500, "Could not allocate the response body"};
		}
		AutoCleaner doc_cleaner([&] { yyjson_mut_doc_free(doc); });

		yyjson_mut_val *root = yyjson_mut_obj(doc);
		yyjson_mut_doc_set_root(doc, root);
		yyjson_mut_obj_add_strcpy(doc, root, "path", source_path.c_str());
		yyjson_mut_obj_add_bool(doc, root, "changed", changed);

		char *json = yyjson_mut_write(doc, 0, nullptr);
		if (!json) {
			return HttpErrorData {InternalServerError_500, "Could not serialize the response body"};
		}
		AutoCleaner json_cleaner([&] { free(json); });

		res.status = 200;
		res.set_content(json, "application/json");
		return nullptr;
	}

	//! Reads a non-empty string field into `value`; the returned error is empty on success.
	static HttpErrorData StringField(yyjson_val *obj, const char *name, std::string &value) {
		yyjson_val *field = yyjson_obj_get(obj, name);
		if (!field || yyjson_get_type(field) != YYJSON_TYPE_STR) {
			return HttpErrorData {BadRequest_400, std::string("Expected '") + name + "' field as string"};
		}
		value = std::string(yyjson_get_str(field), yyjson_get_len(field));
		if (value.empty()) {
			return HttpErrorData {BadRequest_400, std::string("'") + name + "' is empty"};
		}
		return HttpErrorData();
	}

	static Result<FileOperationRequest> Parse(const FileOperation operation, const std::string &request_str) {
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

		std::string path;
		std::string target;
		auto error = StringField(obj, operation == FileOperation::MOVE ? "from" : "path", path);
		if (!error.HasError() && operation == FileOperation::MOVE) {
			error = StringField(obj, "to", target);
		}
		if (error.HasError()) {
			return error;
		}
		return FileOperationRequest(operation, path, target);
	}
};

} // namespace duckdb

#pragma once

#include "http_error_data.hpp"

namespace duckdb {

#define RETURN_IF_ERROR(result)                                                                                        \
	{                                                                                                                  \
		if (result.HasError()) {                                                                                       \
			return result.GetError();                                                                                  \
		}                                                                                                              \
	}
#define RETURN_IF_ERROR_CB(result, cb)                                                                                 \
	{                                                                                                                  \
		if (result.HasError()) {                                                                                       \
			cb(result.GetError());                                                                                     \
			return;                                                                                                    \
		}                                                                                                              \
	}

template <typename...>
struct disjunction : std::false_type {};

template <typename B1>
struct disjunction<B1> : B1 {};

template <typename B1, typename... Bn>
struct disjunction<B1, Bn...> : std::conditional<B1::value, B1, disjunction<Bn...>>::type {};

template <typename T>
class Result {
public:
	template <typename... Args, typename std::enable_if<
	                                !std::is_same<typename std::decay<Args>::type..., HttpErrorData>::value, int>::type = 0>
	Result(Args &&...args) : value(make_uniq<T>(std::forward<Args>(args)...)) {
	}

	Result(HttpErrorData &&_error) : error(std::forward<HttpErrorData>(_error)) {
		D_ASSERT(error.HasError());
	}

	Result(const HttpErrorData &_error) : error(_error) {
		D_ASSERT(error.HasError());
	}

	Result(const StatusCode _code, ErrorData &&_error) : error(_code, _error) {
		D_ASSERT(error.HasError());
	}

	Result(const StatusCode _code, const ErrorData &_error) : error(_code, _error) {
		D_ASSERT(error.HasError());
	}

	bool HasError() const {
		return error.HasError();
	}

	HttpErrorData GetError() {
		D_ASSERT(HasError());
		return error;
	}

	T &GetValue() {
		D_ASSERT(!HasError());
		return value;
	}

	T *operator->() {
		D_ASSERT(!HasError());
		return value.get();
	}

	T &operator*() {
		D_ASSERT(!HasError());
		return value;
	}

private:
	unique_ptr<T> value;
	HttpErrorData error;
};

} // namespace duckdb

#pragma once

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

template <typename T>
class Result {
public:
	Result(T &&_value) : value(make_uniq<T>(std::forward<T>(_value))) {
	}

	Result(ErrorData &&_error) : error(std::forward<ErrorData>(_error)) {
		D_ASSERT(error.HasError());
	}

	Result(const ErrorData &_error) : error(_error) {
		D_ASSERT(error.HasError());
	}

	bool HasError() const {
		return error.HasError();
	}

	ErrorData GetError() {
		D_ASSERT(HasError());
		return error;
	}

	T &GetValue() {
		D_ASSERT(!HasError());
		return value;
	}

	// Implement -> operator to allow accessing results like pointers
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
	ErrorData error;
};

} // namespace duckdb

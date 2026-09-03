# Testing this extension
This directory contains all the tests for this extension. The `sql` directory holds tests that are written as [SQLLogicTests](https://duckdb.org/dev/sqllogictest/intro.html). DuckDB aims to have most its tests in this format as SQL statements, so for the quack extension, this should probably be the goal too.

The root makefile contains targets to build and run all of these tests. To run the SQLLogicTests:
```bash
make test
```
or 
```bash
make test_debug
```

## HTTP tests

`sql/http/` holds tests that go over the wire: they start the server with `start_dash` and
then send real HTTP requests to it with `dash_http`, a table function the extension registers
for this purpose.

```sql
CALL start_dash('127.0.0.1', 39001);

SELECT status, body, headers['Content-Type']
FROM dash_http('POST', 'http://127.0.0.1:39001/api/query',
               headers := MAP {'X-Api-Key': 'abc123'},
               body := '{"query": "SELECT 42", "format": "json"}');
```

`dash_http(method, url)` takes GET, POST or OPTIONS and returns a single row of
`status INTEGER, body VARCHAR, headers MAP(VARCHAR, VARCHAR)`. Named parameters:

| parameter      | meaning                                                                  |
|----------------|--------------------------------------------------------------------------|
| `body`         | request body                                                             |
| `content_type` | content type of the body, `application/json` by default                  |
| `headers`      | request headers                                                          |
| `files`        | file paths to send as a multipart form, with `body` as the `query.json` part |

Only the first value of a repeated response header ends up in `headers`, because a MAP key
has to be unique.

Each file picks its own port and stops the server it started, so the files can run in one
process. A file starts with `CALL stop_dash()` so that a server left behind by an earlier
file, which would make `start_dash` throw, does not fail the whole directory.

`dash_http` is registered only when the extension is built with
`-DDASH_BUILD_TEST_HTTP_CLIENT=ON`, the default, and only in the static extension that the
test binary and the bundled shell link. The loadable extension that ships never has it.

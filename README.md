# Duck_explorer

## Getting started

### Installation

```sql
FROM community INSTALL duck_explorer;
LOAD
duck_explorer;
```

### Usage

```sql
-- Starts the http server
CALL start_duck_explorer('127.0.0.1', 4200)

-- Enable cors (false by default)
CALL start_duck_explorer('127.0.0.1', 4200, enable_cors=true);

-- Require authentication (off by default)
CALL start_duck_explorer('127.0.0.1', 4200, api_key='abc123');

-- Proxy the web UI from a different location
CALL start_duck_explorer('127.0.0.1', 4200, ui_proxy='https://gropaul.github.io/explorer/');
```

Once the _duck_explorer_ is running, access the WebUI by opening http://127.0.0.1:4200 in your browser.

## API Endpoints

| Endpoint | Method | Description                     | Parameters                                                                                                                                                                                                                                 |
|----------|--------|---------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `/`      | GET    | View the integrated web UI.     |                                                                                                                                                                                                                                            |
| `/query` | POST   | Execute an SQL query.           | **Header:** <br> - `X-Api-Key` (optional) – API key for authentication. <br><br> **Body (JSON):** <br> - `query` (string, required) – The SQL query to be executed. <br> - `format` (string, required) – Response format (`compact_json`). |
| `/ping`  | GET    | Check if the server is running. |                                                                                                                                                                                                                                            |

For detailed schema definitions refer to the [API documentation](openapi.yaml).
This also contains instructions on querying and uploading files using the HTTP API. For a reference client
implementation have a look at the [Python test client](./e2e_tests/client.py).

## Development

### Setting up the Repository

Clone the repository and all its submodules

```bash
git clone <your-fork-url>
git submodule update --init --recursive
```

**Build the UI:** Change into the _explorer-ui_ directory and build the UI

```bash
cd explorer-ui
pnpm install --frozen-lockfile
NEXT_PUBLIC_API_URL="" pnpm run build
```

**Generate source files:** Change back to the root directory and generate the source files containing the UI

```bash
python3 scripts/gen_ui_files.py
```

### Setting up CLion

**Opening project:**
Configuring CLion with the extension template requires a little work. Firstly, make sure that the DuckDB submodule is
available.
Then make sure to open `./duckdb/CMakeLists.txt` (so not the top level `CMakeLists.txt` file from this repo) as a
project in CLion.
Now to fix your project path go to
`tools->CMake->Change Project Root`([docs](https://www.jetbrains.com/help/clion/change-project-root-directory.html)) to
set the project root to the root dir of this repo.

**Debugging:**
To set up debugging in CLion, there are two simple steps required. Firstly, in
`CLion -> Settings / Preferences -> Build, Execution, Deploy -> CMake` you will need to add the desired builds (e.g.
Debug, Release, RelDebug, etc). There's different ways to configure this, but the easiest is to leave all empty, except
the `build path`, which needs to be set to `../build/{build type}`. Now on a clean repository you will first need to run
`make {build type}` to initialize the CMake build directory. After running make, you will be able to (re)build from
CLion by using the build target we just created. If you use the CLion editor, you can create a CLion CMake profiles
matching the CMake variables that are described in the makefile, and then you don't need to invoke the Makefile.

The second step is to configure the unittest runner as a run/debug configuration. To do this, go to
`Run -> Edit Configurations` and click `+ -> Cmake Application`. The target and executable should be `unittest`. This
will run all the DuckDB tests. To specify only running the extension specific tests, add `--test-dir ../../.. [sql]` to
the `Program Arguments`. Note that it is recommended to use the `unittest` executable for testing/development within
CLion. The actual DuckDB CLI currently does not reliably work as a run target in CLion.

### Testing

To run the E2E test install all packages necessary:

```bash
pip install -r requirements.txt
```

Then run the test suite:

```bash
pytest e2e_tests
```
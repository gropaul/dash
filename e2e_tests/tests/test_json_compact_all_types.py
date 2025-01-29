from e2e_tests.client import Client, ResponseFormat
from e2e_tests.responses.all_types_compact import ALL_TYPES_COMPACT
from e2e_tests.responses.all_types_json import ALL_TYPES_JSON


def test_json_compact_all_types(http_duck: Client):
    for [response_format, value] in zip(
        [ResponseFormat.JSON, ResponseFormat.COMPACT_JSON], [ALL_TYPES_JSON, ALL_TYPES_COMPACT]
    ):
        res = http_duck.execute_query("FROM test_all_types()", response_format=response_format)
        assert res == value

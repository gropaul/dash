from .client import Client, ResponseFormat
from .responses.all_types_compact import ALL_TYPES_COMPACT


def test_json_compact_all_types(http_duck: Client):
    res = http_duck.execute_query("FROM test_all_types()", response_format=ResponseFormat.COMPACT_JSON)

    assert res == ALL_TYPES_COMPACT

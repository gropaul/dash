def test_absence_of_cors_headers(http_duck):
    res = http_duck.execute_query_raw("SELECT 1")
    assert "Access-Control-Allow-Origin" not in res.headers


def test_presence_of_cors_headers(cors_duck):
    res = cors_duck.execute_query_raw("SELECT 1")
    assert res.headers["Access-Control-Allow-Origin"] == "*"
    assert res.headers["Access-Control-Allow-Methods"] == "POST, GET, OPTIONS"
    assert res.headers["Access-Control-Allow-Headers"] == "X-Api-Key, Content-Type"
    assert res.headers["Access-Control-Allow-Credentials"] == "true"
    assert res.headers["Access-Control-Max-Age"] == "86400"

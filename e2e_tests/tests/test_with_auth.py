from typing import cast

import pytest
from httpx import HTTPStatusError

from e2e_tests.client import Client


def test_with_auth(http_duck_auth: Client):
    res = http_duck_auth.execute_query("SELECT 1")
    assert res["statistics"]["rows"] == 1


def test_without_auth(http_duck_auth: Client):
    with pytest.raises(HTTPStatusError) as e:
        http_duck_auth.with_key(api_key=None).execute_query("SELECT 1")

    assert "Missing" in str(e.value.response.text)
    assert e.value.response.status_code == 401


def test_wrong_auth(http_duck_auth: Client):
    with pytest.raises(HTTPStatusError) as e:
        http_duck_auth.with_key(api_key="__wrong__").execute_query("SELECT 1")

    assert "Invalid" in str(e.value.response.text)
    assert e.value.response.status_code == 401

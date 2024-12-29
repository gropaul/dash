import pytest

from .client import Client


def test_with_auth(http_duck_auth: Client):
    res = http_duck_auth.execute_query("SELECT 1")
    assert res["rows"] == 1


def test_without_auth(http_duck_auth: Client):
    with pytest.raises(Exception) as e:
        http_duck_auth.with_key(api_key=None).execute_query("SELECT 1")

    assert "Missing" in str(e.value)


def test_wrong_auth(http_duck_auth: Client):
    with pytest.raises(Exception) as e:
        http_duck_auth.with_key(api_key="__wrong__").execute_query("SELECT 1")

    assert "Invalid" in str(e.value)

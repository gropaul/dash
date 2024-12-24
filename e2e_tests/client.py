from __future__ import annotations

import time
from enum import Enum

import httpx


class ResponseFormat(Enum):
    COMPACT_JSON = "compact_json"


class Client:
    def __init__(self, url: str, auth_token: str | None = None):
        self._url = url
        self._auth_token = auth_token

    def execute_query(self, sql: str, response_format: ResponseFormat) -> dict:
        headers = {}

        if self._auth_token:
            headers["X-API-Key"] = self._auth_token

        body = {"query": sql, "format": response_format.value}

        with httpx.Client() as client:
            response = client.post(self._url + "/query", headers=headers, json=body)
            response.raise_for_status()
            return response.json()

    def ping(self) -> None:
        with httpx.Client() as client:
            response = client.get(f"{self._url}/ping")
            response.raise_for_status()

    def on_ready(self, timeout=5) -> None:
        end_time = time.time() + timeout
        while time.time() < end_time:
            try:
                self.ping()
                return
            except Exception:
                pass

        raise TimeoutError("Server is not ready")

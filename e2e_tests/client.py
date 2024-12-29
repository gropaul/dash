from __future__ import annotations

import json
import time
from enum import Enum
from pathlib import Path

import httpx
from httpx._types import FileTypes


class ResponseFormat(Enum):
    COMPACT_JSON = "compact_json"


class Client:
    def __init__(self, url: str, api_key: str | None = None):
        self._url = url
        self._api_key = api_key

    def with_key(self, api_key: str | None) -> Client:
        self._api_key = api_key
        return self

    def execute_query(
        self,
        sql: str,
        response_format: ResponseFormat = ResponseFormat.COMPACT_JSON,
        files: list[str | Path] | None = None,
    ) -> dict:
        files = files or []

        headers = {}
        if self._api_key:
            headers["X-API-Key"] = self._api_key

        body = {"query": sql, "format": response_format.value}

        transformed_files: dict[str, FileTypes] = {}
        for file in files:
            file_name = Path(file).name
            transformed_files[file_name] = open(file, "rb")

        transformed_files["query_json"] = (None, json.dumps(body).encode("utf-8"), "application/json")

        with httpx.Client(timeout=120) as client:
            response = client.post(self._url + "/query", headers=headers, json=body, files=transformed_files)
            if not response.is_success:
                raise Exception(response.text)
            return response.json()

    def ping(self, timeout: int | None = None) -> None:
        with httpx.Client(timeout=timeout) as client:
            response = client.get(f"{self._url}/ping")
            response.raise_for_status()

    def on_ready(self, timeout=5) -> None:
        end_time = time.time() + timeout
        while time.time() < end_time:
            try:
                self.ping(timeout=timeout)
                return
            except Exception:
                pass

        raise TimeoutError("Server is not ready")

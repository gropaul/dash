import subprocess
from typing import Iterator

import pytest

from .client import Client
from .const import DEBUG_SHELL, HOST, PORT, API_KEY


def _start_server(start_cmd: str, client: Client) -> Iterator[Client]:
    process = subprocess.Popen(
        [
            DEBUG_SHELL,
        ],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=2 ^ 16,
    )

    # Load the extension
    process.stdin.write("LOAD dash;\n")
    process.stdin.write(start_cmd)

    client.on_ready()
    yield client

    process.kill()


@pytest.fixture
def http_duck() -> Iterator[Client]:
    for client in _start_server(f"CALL start_dash('{HOST}', {PORT});\n", Client(f"http://{HOST}:{PORT}")):
        yield client


@pytest.fixture
def cors_duck() -> Iterator[Client]:
    for client in _start_server(
        f"CALL start_dash('{HOST}', {PORT}, enable_cors=true);\n", Client(f"http://{HOST}:{PORT}")
    ):
        yield client


@pytest.fixture
def http_duck_auth() -> Iterator[Client]:
    for client in _start_server(
        f"CALL start_dash('{HOST}', {PORT}, api_key='{API_KEY}');\n", Client(f"http://{HOST}:{PORT}", API_KEY)
    ):
        yield client

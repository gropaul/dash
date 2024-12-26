import subprocess
from typing import Iterator

import pytest

from .client import Client
from .const import DEBUG_SHELL, HOST, PORT


@pytest.fixture
def http_duck() -> Iterator[Client]:
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
    # process.stdin.write("LOAD duck_explorer;\n")
    # process.stdin.write(f"CALL start_duck_explorer('{HOST}', {PORT});\n")

    client = Client(f"http://{HOST}:{PORT}")
    client.on_ready()
    yield client

    process.kill()

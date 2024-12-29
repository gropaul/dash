import fcntl
import select
import subprocess
from time import sleep

from .client import Client
from .const import DEBUG_SHELL, HOST, PORT

import os
import sys
import select


def get_available_stdout(process):
    """Reads the stdout of the subprocess without blocking."""
    output = b""
    fd = process.stdout.fileno()

    # Set the stdout to non-blocking
    fl = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)

    while True:
        ready, _, _ = select.select([process.stderr], [], [], 0)
        if ready:
            try:
                data = os.read(fd, 4096)
                if not data:
                    break
                output += data
            except BlockingIOError:
                break
        else:
            break
    return output.decode("utf-8", errors="ignore")


def test_failed_server_start():
    process = subprocess.Popen(
        [
            DEBUG_SHELL,
        ],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )

    process.stdin.write("LOAD duck_explorer;\n")

    # Start server with port that is out of range
    process.stdin.write(f"CALL start_duck_explorer('{HOST}', {99999999});\n")
    sleep(1)

    process.stdin.write(f"CALL start_duck_explorer('{HOST}', {PORT}');\n")

    try:
        Client(f"http://{HOST}:{PORT}").on_ready()
    except Exception as e:
        a = process.communicate()
        print(a)
    process.kill()

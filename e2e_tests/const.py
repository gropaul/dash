import os

PROJECT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

DEBUG_SHELL = f"{PROJECT_DIR}/build/debug/duckdb"
RELEASE_SHELL = f"{PROJECT_DIR}/build/release/duckdb"

HOST = "localhost"
PORT = 9999

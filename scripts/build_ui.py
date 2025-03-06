import os
import platform
import tarfile
import zipfile
import urllib.request
import subprocess
import shutil
import sys
import glob

def is_npm_installed():
    """
    Check if 'npm' is on PATH.
    Returns True if found, False otherwise.
    """
    return shutil.which("npm") is not None

def install_node_and_npm(
        version="v18.14.2",
        download_dir="node_download",
        install_dir="node_install"
):
    """
    Downloads and unpacks a portable Node.js + npm into install_dir,
    returning (node_path, npm_path). Works on Linux/macOS/Windows, no shell needed.
    """
    # Figure out which archive name to grab
    system = platform.system().lower()     # "windows", "linux", "darwin"
    machine = platform.machine().lower()   # e.g. "x86_64", "amd64", "arm64", etc.

    # Map machine/CPU names to Node's naming
    # (You may need additional logic for other architectures)
    if machine in ("x86_64", "amd64"):
        arch = "x64"
    elif "arm64" in machine or "aarch64" in machine:
        arch = "arm64"
    elif "arm" in machine:
        # This is simplistic: Node has multiple ARM variants (armv7l, armv6l, etc.)
        # Adjust as needed for your environment.
        arch = "armv7l"
    else:
        raise RuntimeError(f"Unsupported CPU architecture: {machine}")

    base_url = f"https://nodejs.org/dist/{version}"

    # We’ll pick .tar.gz for Linux & macOS, .zip for Windows
    if system == "windows":
        archive_name = f"node-{version}-win-{arch}.zip"
    elif system == "darwin":
        archive_name = f"node-{version}-darwin-{arch}.tar.gz"
    else:  # assume Linux
        archive_name = f"node-{version}-linux-{arch}.tar.gz"

    download_url = f"{base_url}/{archive_name}"
    local_archive = os.path.join(download_dir, archive_name)

    os.makedirs(download_dir, exist_ok=True)
    os.makedirs(install_dir, exist_ok=True)

    # 1) Download the archive (no shell commands, just urllib)
    print(f"Downloading {download_url} ...")
    with urllib.request.urlopen(download_url) as response:
        data = response.read()

    # 2) Save archive locally just in case
    with open(local_archive, "wb") as f:
        f.write(data)

    # 3) Extract into install_dir
    print(f"Extracting {archive_name} ...")
    if system == "windows":
        with zipfile.ZipFile(local_archive, "r") as zf:
            zf.extractall(install_dir)
        extracted_folder_name = f"node-{version}-win-{arch}"
        # On Windows, Node and npm are at the top level within that folder.
        node_executable = os.path.join(install_dir, extracted_folder_name, "node.exe")
        npm_executable  = os.path.join(install_dir, extracted_folder_name, "npm.cmd")
    else:
        with tarfile.open(local_archive, "r:gz") as tf:
            tf.extractall(path=install_dir)
        extracted_folder_name = (
                f"node-{version}-" + ("linux" if system == "linux" else "darwin") + f"-{arch}"
        )
        node_executable = os.path.join(install_dir, extracted_folder_name, "bin", "node")
        npm_executable  = os.path.join(install_dir, extracted_folder_name, "bin", "npm")

    if not os.path.exists(node_executable):
        raise RuntimeError(f"Node binary not found at {node_executable}")
    if not os.path.exists(npm_executable):
        raise RuntimeError(f"npm binary not found at {npm_executable}")

    print(f"Node installed to: {node_executable}")
    print(f"npm installed to:  {npm_executable}")

    return node_executable, npm_executable

def install_tools():
    # 1) Check if npm is already installed
    if is_npm_installed():
        print("npm is already installed on the system; no installation needed.")
        return

    # 2) Otherwise, download and unpack Node (which includes npm)
    node_path, npm_path = install_node_and_npm(
        version="v18.14.2",         # Or choose a different Node version
        download_dir="node_dl",     # Scratch folder for the archive
        install_dir="node_install"  # Final install location
    )

    # 3) Demonstration: run npm to verify
    result = subprocess.run([npm_path, "--version"], capture_output=True, text=True)
    if result.returncode == 0:
        print("npm version:", result.stdout.strip())
    else:
        print("Failed to run npm from the new installation.")

# test
def build_ui():
    # go to the dash-ui directory
    dash_ui_dir = os.path.join(os.path.dirname(__file__), "..", "dash-ui")
    # check if the dash-ui directory exists
    if not os.path.exists(dash_ui_dir):
        print("dash-ui directory not found at", dash_ui_dir)
        print("Please clone the dash-ui repository in the root directory of the extension repository.")
        sys.exit(1)

    # change the current working directory to dash-ui
    os.chdir(dash_ui_dir)

    # check if npm is installed
    if not shutil.which("npm"):
        print("npm not found. Please install npm.")
        sys.exit(1)

    # check if pnpm is installed, if not install it
    if not shutil.which("pnpm"):
        print("pnpm not found. Installing pnpm...")
        os.system("npm install -g pnpm")

    # install dependencies
    print("Installing dependencies...")
    os.system("pnpm install")

    # build the UI
    print("Building UI...")
    os.system("pnpm run build:extension")

root_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
target_file = os.path.join(root_dir, "src", "gen", "files.cpp")

CONTENT_TEMPLATE = """
#pragma once
#include "files.hpp"
namespace duckdb {
File %var_name% = {
     // Content
     %content%,    //
     R"DELIM(%content_type%)DELIM", //
     R"DELIM(%path%)DELIM", //
};
}
"""

FILE_TEMPLATE = """
#include "files.hpp"

#include "duckdb/common/optional_ptr.hpp"
#include "duckdb/common/string_util.hpp"

%include%

#include <map>

namespace duckdb { 
std::vector<File> files = {
	%content%
};

optional_ptr<File> GetFile(Path path, const bool try_resolve_404) {
	if (path.empty() || path == "/") {
		path = "index.html";
	}

	// Append / before and after
	path = "/" + path + "/";
	path = StringUtil::Replace(path, "//", "/");

	for (auto &file : files) {
		if (file.path == path) {
			return file;
		}
	}

	if (try_resolve_404) {
		GetFile("/404.html/", false);
	}

	return nullptr;
}

}
"""


def normalize_path(path: str) -> str:
    path = f"/{path}/"
    return path.replace("//", "/")


def get_content_type(file: str) -> str:
    content_types = {
        "html": "text/html",
        "css": "text/css",
        "js": "application/javascript",
        "png": "image/png",
        "jpg": "image/jpeg",
        "ico": "image/x-icon",
        "woff": "font/woff",
        "woff2": "font/woff2",
        "ttf": "font/ttf",
        "svg": "image/svg+xml",
        "json": "application/json",
        "txt": "text/plain",
    }
    extension = file.split(".")[-1]
    return content_types[extension]


def generate_ui_files():
    base_path = os.path.join(root_dir, "dash-ui", "out")
    target_dir = os.path.dirname(target_file)
    if os.path.exists(target_dir):
        shutil.rmtree(target_dir)
    os.makedirs(target_dir)

    if not os.path.exists(base_path):
        raise Exception(f"Path {base_path} does not exist. Build the UI first.")

    files = glob.glob(os.path.join(base_path, "**", "*"), recursive=True)

    content_list: list[str] = []

    for file in files:
        if not os.path.isfile(file):
            continue
        content = open(
            file,
            "rb",
        ).read()

        # Goal: {0x33, 0x55}
        transformed_content = "{"
        for byte in content:
            transformed_content += f"0x{byte:02x}, "
        transformed_content += "}"

        path = normalize_path(file.replace(base_path, ""))

        path_as_name = path.replace("/", "_").replace(".", "_").replace("-", "_")
        final_path = target_dir + "/" + path_as_name + ".hpp"

        templated = (
            CONTENT_TEMPLATE.replace("%path%", path)
            .replace("%content%", transformed_content)
            .replace("%content_type%", get_content_type(file))
            .replace("%var_name%", path_as_name.upper())
        )

        # create path on the way to the file
        os.makedirs(os.path.dirname(final_path), exist_ok=True)

        with open(final_path, "w") as f:
            f.write(templated)
        content_list.append(path_as_name)

    content = (
        FILE_TEMPLATE.replace("%content%", ",".join(content_list).upper())
        .lstrip()
        .replace("%include%", "\n".join([f'#include "{f}.hpp"' for f in content_list]))
    )

    with open(target_file, "w") as f:
        f.write(content)


def main():
    print("*** Building UI ***")
    install_tools()
    build_ui()
    generate_ui_files()
    print("*** Done ***")


print("Building UI...")
main()
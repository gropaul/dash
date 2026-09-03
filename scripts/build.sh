#!/usr/bin/env bash
#
# Build the `dash` DuckDB extension end-to-end from a fresh clone.
#
# This handles everything needed to go from `git clone` to a loadable
# extension binary:
#   1. Initialises the required git submodules (duckdb, extension-ci-tools).
#   2. Sets up vcpkg, which provides the extension's OpenSSL dependency.
#   3. Builds the extension via the standard Makefile target.
#
# It is designed to work both locally and in CI:
#   - In CI, the reusable duckdb/extension-ci-tools workflow exports
#     VCPKG_ROOT and VCPKG_TOOLCHAIN_PATH; this script picks those up.
#   - Locally, if neither is set, it bootstraps vcpkg into ./vcpkg, pinned to
#     the same commit CI uses, so the dependency graph matches.
#
# It deliberately does NOT run scripts/build_ui.py: the generated UI sources
# under src/gen/ are already committed, so the `dash-ui` submodule and the
# Node/pnpm toolchain are not required to build the extension.
#
# Usage:
#   scripts/build.sh [release|debug]      # build type, defaults to release
#
# Environment overrides:
#   VCPKG_TOOLCHAIN_PATH   Path to an existing vcpkg.cmake toolchain file.
#   VCPKG_ROOT             Path to an existing vcpkg checkout.
#
# vcpkg commit kept in sync with extension-ci-tools' _extension_distribution.yml.
VCPKG_COMMIT="84bab45d415d22042bd0b9081aea57f362da3f35"
set -euo pipefail

# --- Locate the repository root (parent of this script's directory) ---------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${REPO_ROOT}"

BUILD_TYPE="${1:-release}"
if [[ "${BUILD_TYPE}" != "release" && "${BUILD_TYPE}" != "debug" ]]; then
    echo "error: build type must be 'release' or 'debug' (got '${BUILD_TYPE}')" >&2
    exit 1
fi

log() { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }

# --- 1. Check required tools ------------------------------------------------
log "Checking required tools"
MISSING=()
for tool in git cmake make; do
    command -v "${tool}" >/dev/null 2>&1 || MISSING+=("${tool}")
done
if [[ ${#MISSING[@]} -gt 0 ]]; then
    echo "error: missing required tools: ${MISSING[*]}" >&2
    echo "       please install them and re-run this script." >&2
    exit 1
fi

# --- 2. Initialise required submodules --------------------------------------
# Only duckdb and extension-ci-tools are needed to build the extension.
# dash-ui is intentionally skipped (see header comment).
log "Initialising submodules (duckdb, extension-ci-tools)"
git submodule update --init --recursive duckdb extension-ci-tools

# --- 3. Set up vcpkg (needed for the OpenSSL dependency) --------------------
if [[ -n "${VCPKG_TOOLCHAIN_PATH:-}" ]]; then
    log "Using existing VCPKG_TOOLCHAIN_PATH=${VCPKG_TOOLCHAIN_PATH}"
elif [[ -n "${VCPKG_ROOT:-}" && -f "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ]]; then
    export VCPKG_TOOLCHAIN_PATH="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    log "Using vcpkg from VCPKG_ROOT=${VCPKG_ROOT}"
else
    VCPKG_DIR="${REPO_ROOT}/vcpkg"
    if [[ ! -f "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake" ]]; then
        log "Bootstrapping vcpkg into ${VCPKG_DIR} (commit ${VCPKG_COMMIT})"
        mkdir -p "${VCPKG_DIR}"
        git -C "${VCPKG_DIR}" init -q
        git -C "${VCPKG_DIR}" remote get-url origin >/dev/null 2>&1 \
            || git -C "${VCPKG_DIR}" remote add origin https://github.com/microsoft/vcpkg.git
        git -C "${VCPKG_DIR}" fetch --depth 1 origin "${VCPKG_COMMIT}"
        git -C "${VCPKG_DIR}" checkout -q "${VCPKG_COMMIT}"
        "${VCPKG_DIR}/bootstrap-vcpkg.sh" -disableMetrics
    else
        log "Using previously bootstrapped vcpkg in ${VCPKG_DIR}"
    fi
    export VCPKG_ROOT="${VCPKG_DIR}"
    export VCPKG_TOOLCHAIN_PATH="${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake"
fi

# --- 4. Build the extension -------------------------------------------------
log "Building the dash extension (${BUILD_TYPE})"
make "${BUILD_TYPE}"

# --- 5. Done ----------------------------------------------------------------
EXT_BINARY="${REPO_ROOT}/build/${BUILD_TYPE}/extension/dash/dash.duckdb_extension"
log "Build complete"
if [[ -f "${EXT_BINARY}" ]]; then
    echo "Extension binary: ${EXT_BINARY}"
fi
echo "DuckDB CLI:       ${REPO_ROOT}/build/${BUILD_TYPE}/duckdb"
echo
echo "Try it out:"
echo "  ./build/${BUILD_TYPE}/duckdb -unsigned -c \"LOAD 'dash'; PRAGMA dash;\""

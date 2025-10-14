#!/usr/bin/env bash

set -euo pipefail

usage() {
    cat <<EOF
Usage: $0 [--bin BIN | --bins DIR]... [--place-bins PATH] --emit-tar TARBALL [options]

Targets (at least one):
    --bin PATH        Add a single binary (repeatable)
    --bins DIR        Add all executables in DIR (non-recursive)

Outputs:
    --dest PATH       Destination where to write the rootfs (required)
    --emit-tar PATH   Write resulting tarball here (required)

Options:
    -h|--help         Show help
EOF
}

declare -a BIN_LIST=() BINS_DIRS=()
EMIT_TAR=""
INCLUDE_GLIBC=0
QUIET=0
ROOT=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --bin) BIN_LIST+=("$2"); shift 2;;
        --bins) BINS_DIRS+=("$2"); shift 2;;
        --dest) ROOT="$2"; shift 2;;
        --emit-tar) EMIT_TAR="$2"; shift 2;;
        --quiet) QUIET=1; shift;;
        -h|--help) usage; exit 0;;
        *) echo "Unknown arg: $1" >&2; usage; exit 1;;
    esac
done
[[ -n "$EMIT_TAR" ]] || { echo "ERROR: --emit-tar is required" >&2; exit 1; }
[[ -n "$ROOT" ]] || { echo "ERROR: --dest is required" >&2; exit 1; }

# Expand DIRs to bins
for d in "${BINS_DIRS[@]}"; do
    [[ -d "$d" ]] || continue
    while IFS= read -r -d '' f; do BIN_LIST+=("$f"); done < <(find "$d" -maxdepth 1 -type f -perm -111 -print0)
done

# Dedup + filter
declare -A SEEN; declare -a BINS=()
for b in "${BIN_LIST[@]}"; do
  [[ -f "$b" ]] || continue
  [[ -x "$b" ]] || continue
  [[ -n "${SEEN[$b]:-}" ]] || { SEEN["$b"]=1; BINS+=("$b"); }
done
[[ ${#BINS[@]} -gt 0 ]] || { echo "ERROR: no binaries provided" >&2; exit 1; }

# Glibc skip patterns (sonames) — safe to exclude on Ubuntu base
GLIBC_SKIP_REGEX='(^/lib64/ld-linux.*|/ld-linux.*|/libc\.so.*|/libm\.so.*|/libdl\.so.*|/libpthread\.so.*|/librt\.so.*|/libresolv\.so.*|/libnss_.*\.so.*|/libanl\.so.*|/libcrypt\.so.*)'

mkdir -p "$ROOT"

# Return the real path of a lib (resolving symlinks)
realpath_lib() {
    readlink -f "$1" 2>/dev/null || echo "$1";
}

# Extract SONAME from a shared object
soname_of() {
    readelf -d "$1" 2>/dev/null | awk '/SONAME/ {gsub(/[\[\]]/,"",$5); print $5; exit}'
}

normalize_lib_path() {
    local p="$1"
    if [[ "$p" =~ ^/lib/x86_64-linux-gnu/ ]]; then
        echo "/usr${p}"
    else
        echo "$p"
    fi
}

# Copy lib path (symlink or real), its real target, and ensure SONAME symlink exists
install_lib() {
    local src="$1"
    [[ -e "$src" ]] || return 0

    # Sesolve symlink target to get the real file
    local real; real="$(readlink -f "$src" 2>/dev/null || true)"
    [[ -n "$real" ]] || real="$src"

    # Destination paths normalized under /usr/lib/...
    local dst="$(normalize_lib_path "$src")"
    local dst_real="$(normalize_lib_path "$real")"

    mkdir -p "$ROOT$(dirname "$dst")"
    cp -a "$src" "$ROOT$dst"

    mkdir -p "${ROOT}$(dirname "$dst_real")"
    cp -a "$real" "${ROOT}${dst_real}"

    # Ensure SONAME symlink exists next to the REAL file
    local so
    so="$(readelf -d "$real" 2>/dev/null | awk '/SONAME/ {gsub(/[\[\]]/,"",$5); print $5; exit}')"
    if [[ -n "$so" ]]; then
        local d="$(dirname "$dst_real")"
        if [[ ! -e "${ROOT}${d}/${so}" ]]; then
            ln -s "$(basename "$dst_real")" "${ROOT}${d}/${so}"
        fi
    fi
}

maybe_strip() {
    local path="$1"
    if command -v strip >/dev/null 2>&1; then
        strip --strip-unneeded "$path" || true
    else
        echo " skipping strip for $path"
    fi
}

# Collect libs
for bin in "${BINS[@]}"; do
    echo "collect: $bin"
    # Collect non-glibc libs reported by ldd
    while IFS= read -r lib; do
        [[ -z "$lib" ]] && continue
        if [[ ! -e "$lib" ]]; then
            # Fallback: lib is likely a SONAME (e.g., libcurl.so.4). Resolve with ldconfig.
            if [[ "$lib" != /* ]]; then
                resolved="$(ldconfig -p | awk -v soname="$lib" '$1 == soname { print $NF; exit }')"
            [[ -n "$resolved" && -e "$resolved" ]] && lib="$resolved" || { echo "WARN: not found: $lib"; continue; }
            else
                echo "WARN: missing path: $lib"; continue
            fi
        fi

        # Avoid copying glibc libs that can mess-up when we overlay them on
        # the new rootfs
        if [[ "$lib" =~ $GLIBC_SKIP_REGEX ]]; then
            echo "  skip glibc: $lib"
        else
            install_lib "$lib"
        fi
    done < <(ldd "$bin" | awk '/=>/ {print $3} /^\/[^ ]+/ {print $1}' | sort -u)
done

# Place your binaries
for bin in "${BINS[@]}"; do
    echo "copying binary: $bin"

    # Strip before copying the binary
    maybe_strip "$bin"

    real_bin=$(realpath_lib "$bin")
    mkdir -p "$ROOT$(dirname $real_bin)"
    cp "$real_bin" "$ROOT$real_bin"
done

# Pack tar
tar -C "$ROOT" -cf "$EMIT_TAR" .
rm -rf "$ROOT"
echo "wrote tar: $EMIT_TAR"

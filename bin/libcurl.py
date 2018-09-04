# ------------------------------------------------
# Script to download and compile libcurl to WASM
# ------------------------------------------------

from os import mkdir
from os.path import join, dirname, realpath, exists

from requests import get
from subprocess import call

CONFIG_FLAGS = [
    "--without-ssl",
    "--disable-ares",
    "--disable-cookies",
    "--disable-crypto-auth",
    "--disable-ipv6",
    "--disable-manual",
    "--disable-proxy",
    "--disable-verbose",
    "--disable-versioned-symbols",
    "--without-libidn",
    "--without-librtmp",
    "--without-ssl",
    "--without-zlib"
]

PROJ_ROOT = dirname(dirname(realpath(__file__)))

EMSCRIPTEN_DIR = "/usr/local/code/emsdk/emscripten/1.38.10/"
EMCONFIGURE = join(EMSCRIPTEN_DIR, "emconfigure")
EMMAKE = join(EMSCRIPTEN_DIR, "emmake")

CURL_EXTRACT_DIR = "/tmp/curl-curl-7_61_0"
LIBCURL_URL = "https://github.com/curl/curl/archive/curl-7_61_0.tar.gz"
WASM_DIR = join(PROJ_ROOT, "lib", "libcurl")

if __name__ == "__main__":
    with open("/tmp/libcurl.tar.gz", "wb") as fh:
        response = get(LIBCURL_URL)
        fh.write(response.content)

    # Extract
    call(["tar", "-xvf", "libcurl.tar.gz"], cwd="/tmp")

    # Configure
    call(["./buildconf"], cwd=CURL_EXTRACT_DIR)
    config_cmd = [EMCONFIGURE, "./configure"]
    config_cmd.extend(CONFIG_FLAGS)
    call(config_cmd, cwd=CURL_EXTRACT_DIR)

    # Make
    call([EMMAKE, "make"], cwd=CURL_EXTRACT_DIR)

    if not exists(WASM_DIR):
        mkdir(WASM_DIR)

    # Copy output into place
    call(["cp", "-r", join(CURL_EXTRACT_DIR, "include"), WASM_DIR])
    call(["cp", join(CURL_EXTRACT_DIR, "lib", ".libs", "libcurl.so"), WASM_DIR])

    print("Output at {}".format(WASM_DIR))

from os import mkdir
from os.path import exists, join
from shutil import rmtree
from subprocess import call

from requests import get

from compile.env import WASM_LIB_DIR, ENV, TARGET_TRIPLE

CONFIG_FLAGS = [
    "--target={}".format(TARGET_TRIPLE),
    "--host=wasm32",
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

CURL_EXTRACT_DIR = "/tmp/curl-curl-7_61_0"
LIBCURL_URL = "https://github.com/curl/curl/archive/curl-7_61_0.tar.gz"
WASM_DIR = join(WASM_LIB_DIR, "libcurl")


def compile_lib(args):
    if args.name == "curl":
        compile_libcurl()
    else:
        raise RuntimeError("Unrecognised lib name: {}".format(args.name))


def compile_libcurl():
    """
    Compiles libcurl to wasm
    """

    if not exists(CURL_EXTRACT_DIR):
        # Download the file
        with open("/tmp/libcurl.tar.gz", "wb") as fh:
            response = get(LIBCURL_URL)
            fh.write(response.content)

        # Extract
        call(["tar", "-xvf", "libcurl.tar.gz"], cwd="/tmp")

    # Configure
    call(["./buildconf"], cwd=CURL_EXTRACT_DIR, env=ENV, shell=True)
    config_cmd = [
        "./configure",
        *CONFIG_FLAGS
    ]
    config_cmd = " ".join(config_cmd)
    print("Calling: {}".format(config_cmd))
    call(config_cmd, cwd=CURL_EXTRACT_DIR, env=ENV, shell=True)

    # Make
    call("make", cwd=CURL_EXTRACT_DIR, env=ENV, shell=True)

    if exists(WASM_LIB_DIR):
        rmtree(WASM_LIB_DIR)
    mkdir(WASM_LIB_DIR)

    # Copy output into place
    call(["cp", "-r", join(CURL_EXTRACT_DIR, "include"), WASM_DIR])
    call(["cp", join(CURL_EXTRACT_DIR, "lib", ".libs", "libcurl.so"), WASM_DIR])

    print("Output at {}".format(WASM_DIR))

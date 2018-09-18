from os.path import exists, join
from shutil import rmtree
from subprocess import call

from requests import get

from compile.env import WASM_LIB_DIR, CONFIG_TARGET, ENV_STR, ENV_DICT

CONFIG_FLAGS = [
    ENV_STR,
    "--target={}".format(CONFIG_TARGET),
    "--host={}".format(CONFIG_TARGET),
    "--prefix={}".format(WASM_LIB_DIR),
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

    # Clear output directory
    if exists(WASM_DIR):
        rmtree(WASM_DIR)

    # Configure
    call(["./buildconf"], cwd=CURL_EXTRACT_DIR, env=ENV_DICT, shell=True)
    config_cmd = [
        "./configure",
        *CONFIG_FLAGS
    ]
    config_cmd = " ".join(config_cmd)
    print(config_cmd)
    res = call(config_cmd, cwd=CURL_EXTRACT_DIR, env=ENV_DICT, shell=True)
    if res != 0:
        raise RuntimeError("Configure command failed")

    # Make
    res = call("make", cwd=CURL_EXTRACT_DIR, env=ENV_DICT, shell=True)
    if res != 0:
        raise RuntimeError("Make failed")

    res = call("make install", cwd=CURL_EXTRACT_DIR, env=ENV_DICT, shell=True)
    if res != 0:
        raise RuntimeError("Make install failed")

    print("Output at {}".format(WASM_DIR))

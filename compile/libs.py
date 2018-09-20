from os.path import exists, join
from subprocess import call

from requests import get

from compile.env import CONFIG_TARGET, ENV_STR, ENV_DICT, SYSROOT

CONFIG_FLAGS = [
    "--target={}".format(CONFIG_TARGET),
    "--host={}".format(CONFIG_TARGET),
    "--prefix={}".format(SYSROOT),
    "--disable-threaded-resolver",
    "--without-winssl",
    "--without-darwinssl",
]

CURL_EXTRACT_DIR = "/tmp/curl-curl-7_61_0"
LIBCURL_URL = "https://github.com/curl/curl/archive/curl-7_61_0.tar.gz"


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

    # Buildconf
    if not exists(join(CURL_EXTRACT_DIR, "configure")):
        call(["./buildconf"], cwd=CURL_EXTRACT_DIR, env=ENV_DICT, shell=True)

    # Configure
    config_cmd = [
        "./configure",
        ENV_STR,
        *CONFIG_FLAGS
    ]
    config_cmd = " ".join(config_cmd)
    print(config_cmd)
    res = call(config_cmd, cwd=CURL_EXTRACT_DIR, env=ENV_DICT, shell=True)
    if res != 0:
        raise RuntimeError("Configure command failed")

    # Make (note we only want to make the lib)
    make_dir = join(CURL_EXTRACT_DIR)
    res = call("make", cwd=make_dir, env=ENV_DICT, shell=True)
    if res != 0:
        raise RuntimeError("Make failed")

    res = call("make install", cwd=make_dir, env=ENV_DICT, shell=True)
    if res != 0:
        raise RuntimeError("Make install failed")

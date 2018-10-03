from os import mkdir
from os.path import exists, join
from subprocess import call

from requests import get

from compile.env import CONFIG_TARGET, ENV_STR, SYSROOT, PROJ_ROOT

CONFIG_FLAGS = [
    "--target={}".format(CONFIG_TARGET),
    "--host={}".format(CONFIG_TARGET),
    "--prefix={}".format(SYSROOT),
    "--disable-threaded-resolver",
    "--without-winssl",
    "--without-darwinssl",
]

LIBCURL_URL = "https://github.com/curl/curl/archive/curl-7_61_0.tar.gz"
DOWNLOAD_DIR = join(PROJ_ROOT, "download")
CURL_EXTRACT_DIR = join(DOWNLOAD_DIR, "curl-curl-7_61_0")


def compile_lib(args):
    if not exists(DOWNLOAD_DIR):
        mkdir(DOWNLOAD_DIR)

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
        with open(join(DOWNLOAD_DIR, "libcurl.tar.gz"), "wb") as fh:
            response = get(LIBCURL_URL)
            fh.write(response.content)

        # Extract
        call(["tar", "-xvf", "libcurl.tar.gz"], cwd=DOWNLOAD_DIR)

    # Buildconf
    buildconf_cmd = ["./buildconf"]
    res = call(buildconf_cmd, shell=True, cwd=CURL_EXTRACT_DIR)
    if res != 0:
        raise RuntimeError("Buildconf command failed")

    # Configure
    config_cmd = [
        "./configure",
        ENV_STR,
        *CONFIG_FLAGS
    ]
    config_cmd_str = " ".join(config_cmd)
    res = call(config_cmd_str, shell=True, cwd=CURL_EXTRACT_DIR)
    if res != 0:
        raise RuntimeError("Configure command failed")

    # Make
    make_cmd = ["make"]
    res = call(make_cmd, shell=True, cwd=CURL_EXTRACT_DIR)
    if res != 0:
        raise RuntimeError("Make failed")

    res = call("make install", shell=True, cwd=CURL_EXTRACT_DIR)
    if res != 0:
        raise RuntimeError("Make install failed")

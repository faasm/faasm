import copy
import os
import shutil
from copy import copy
from os import mkdir
from os.path import exists
from os.path import join
from subprocess import call

from invoke import task

from tasks.download import FAASM_HOME, clone_proj
from tasks.env import FAASM_RUNTIME_ROOT, EMSCRIPTEN_DIR, EMSCRIPTEN_ENV_DICT, NATIVE_ENV_DICT, ENV_STR, CONFIG_TARGET, \
    CONFIG_HOST, SYSROOT

# Note: this must be the dir that contains "include" and "lib" into the runtime root
PY_EMSCRIPTEN_INSTALL_DIR = join(FAASM_HOME, "cpython-emscripten", "installs", "python-3.5.2")

# Subset of files to make available to the python runtime
PYTHON_LIB_FILES = [
    "_collections_abc.py",
    "_dummy_thread.py",
    "_sitebuiltins.py",
    "_sysconfigdata.py",
    "_weakrefset.py",
    "abc.py",
    "codecs.py",
    "collections/__init__.py",
    "collections/abc.py",
    "contextlib.py",
    "encodings/__init__.py",
    "encodings/aliases.py",
    "encodings/cp437.py",
    "encodings/latin_1.py",
    "encodings/utf_8.py",
    "functools.py",
    "genericpath.py",
    "heapq.py",
    "importlib/__init__.py",
    "importlib/_bootstrap.py",
    "importlib/_bootstrap_external.py",
    "importlib/abc.py",
    "importlib/machinery.py",
    "importlib/util.py",
    "io.py",
    "keyword.py",
    "operator.py",
    "os.py",
    "pkgutil.py",
    "posixpath.py",
    "reprlib.py",
    "runpy.py",
    "site.py",
    "stat.py",
    "sysconfig.py",
    "types.py",
    "warnings.py",
    "weakref.py",
]


@task
def build_python_emscripten(ctx):
    proj_dir = clone_proj("https://github.com/Shillaker/cpython-emscripten", "cpython-emscripten")

    # Make sure emscripten is present
    assert exists(EMSCRIPTEN_DIR), "Must have emscripten ready"

    # Run the emscripten python build
    current_path = os.environ["PATH"]
    em_path = "{}:{}".format(current_path, EMSCRIPTEN_DIR)
    env_vars = copy(EMSCRIPTEN_ENV_DICT)
    env_vars["PATH"] = em_path

    make_dir = join(proj_dir, "3.5.2")
    call("make", cwd=make_dir, shell=True, env=env_vars)


@task
def set_up_python_root(ctx):
    # Create root if necessary
    if not exists(FAASM_RUNTIME_ROOT):
        raise RuntimeError("Must create runtime root at {}".format(FAASM_RUNTIME_ROOT))

    # We need to copy the relevant subset of files from the lib dir
    dest_lib_dir = join(FAASM_RUNTIME_ROOT, "lib")
    if not exists(dest_lib_dir):
        mkdir(dest_lib_dir)

    dest_lib_dir = join(dest_lib_dir, "python3.5")
    if not exists(dest_lib_dir):
        mkdir(dest_lib_dir)

    src_lib_dir = join(PY_EMSCRIPTEN_INSTALL_DIR, "lib", "python3.5")
    for filename in PYTHON_LIB_FILES:
        # Copy everything from the build dir into the runtime root
        file_path = join(src_lib_dir, filename)
        shutil.copy(file_path, dest_lib_dir)


@task
def build_python_host(ctx):
    # -----------------------------------
    # Build for native host (used by cross-compile build steps)
    # -----------------------------------

    host_proj = clone_proj("https://github.com/Shillaker/cpython", "cpython-host", branch="wasm")
    host_build_dir = join(host_proj, "host_build")

    call("./configure --prefix={}".format(host_build_dir),
         shell=True, cwd=host_proj, env=NATIVE_ENV_DICT)

    # call("make regen-grammar", shell=True, cwd=host_proj, env=NATIVE_ENV_DICT)
    call("make python Parser/pgen", shell=True, cwd=host_proj, env=NATIVE_ENV_DICT)
    call("make install", shell=True, cwd=host_proj, env=NATIVE_ENV_DICT)


@task
def build_python(ctx):
    # Note, full python version is 3.6.7
    host_proj = join(FAASM_HOME, "cpython-host")
    host_python = join(host_proj, "host_build", "bin", "python3")
    host_pgen = join(host_proj, "Parser", "pgen")
    python_lib = "libpython3.6.a"

    # -----------------------------------
    # Configure cross-compile
    # -----------------------------------
    target_proj = clone_proj("https://github.com/Shillaker/cpython", "cpython-wasm", branch="wasm")

    config_cmd = [
        "CONFIG_SITE=./config.site",
        "READELF=true",
        "./configure",
        ENV_STR,
        "--without-threads",
        "--without-pymalloc",
        "--disable-shared",
        "--disable-ipv6",
        "--without-gcc",
        "--target={}".format(CONFIG_TARGET),
        "--host={}".format(CONFIG_HOST),
        "--build={}".format(CONFIG_TARGET),
        "--prefix={}".format(SYSROOT),
    ]

    # Run configuration
    config_cmd_str = " ".join(config_cmd)
    call(config_cmd_str, shell=True, cwd=target_proj)

    # Copy setup local file into place
    call("cp wasm32/Modules_Setup.local Modules/Setup.local", shell=True, cwd=target_proj)

    # -----------------------------------
    # Build static zlib
    # -----------------------------------

    zlib_config_cmd = [
        ENV_STR,
        "./configure",
        "--static",
    ]
    zlib_config_cmd_str = " ".join(zlib_config_cmd)
    call(zlib_config_cmd_str, cwd=join(target_proj, "Modules", "zlib"), shell=True)

    base_make_cmd = [
        "make",
        "HOSTPYTHON={}".format(host_python),
        "HOSTPGEN={}".format(host_pgen),
        "CROSS_COMPILE=yes",
    ]

    # -----------------------------------
    # Build core Python lib
    # -----------------------------------
    make_cmd_a = copy.copy(base_make_cmd)
    make_cmd_a.append(python_lib)

    # Invoke the make command
    call(" ".join(make_cmd_a), shell=True, cwd=target_proj)

    # -----------------------------------
    # Build all Python standard modules
    # -----------------------------------

    # Remove any of the dependencies of the libinstall target
    call("sed -i -e 's/libinstall:.*/libinstall:/' Makefile;", shell=True, cwd=target_proj)

    # Build make command
    make_cmd_b = copy.copy(base_make_cmd)
    make_cmd_b.extend([
        "PYTHON_FOR_BUILD={}".format(host_python),
        "inclinstall",
        python_lib,
        "libinstall"
    ])

    call(" ".join(make_cmd_b), shell=True, cwd=target_proj)

    # -----------------------------------
    # Tidy up
    # -----------------------------------

    # Put lib in place
    call("cp {} {}".format(python_lib, join(SYSROOT, "lib")), shell=True, cwd=target_proj)

    # Put sysconfig in place
    # host_sysconfig = join(host_build_dir, "lib", "python3.6", "_sysconfigdata_m_linux_x86_64-linux-gnu.py")
    # sysconfig_dest = join(SYSROOT, "lib", "python3.6", "_sysconfigdata__wasm32_x86_64-linux-gnu.py")
    # os.copy(host_sysconfig, sysconfig_dest)

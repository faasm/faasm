import copy
from copy import copy
from os.path import exists
from os.path import join
from subprocess import call

from invoke import task

from tasks.compile import check_correct_emscripten
from tasks.download import FAASM_HOME, clone_proj
from tasks.env import NATIVE_ENV_DICT, ENV_STR, CONFIG_TARGET, \
    CONFIG_HOST, SYSROOT, PY_EMSCRIPTEN_DIR, PROJ_ROOT, EMSCRIPTEN_DIR, PYODIDE_ROOT, FAASM_RUNTIME_ROOT

# Note: this must be the dir that contains "include" and "lib" into the runtime root
EMSCRIPTEN_CPYTHON_INSTALL_DIR = join(PROJ_ROOT, "cpython-emscripten", "installs", "python-3.5.2")

# Subset of files to make available to the python runtime
EMSCRIPTEN_PYTHON_LIB_FILES = [
    "lib/python3.5/_collections_abc.py",
    "lib/python3.5/_dummy_thread.py",
    "lib/python3.5/_sitebuiltins.py",
    "lib/python3.5/_sysconfigdata.py",
    "lib/python3.5/_weakrefset.py",
    "lib/python3.5/abc.py",
    "lib/python3.5/codecs.py",
    "lib/python3.5/collections/__init__.py",
    "lib/python3.5/collections/abc.py",
    "lib/python3.5/contextlib.py",
    "lib/python3.5/encodings/__init__.py",
    "lib/python3.5/encodings/aliases.py",
    "lib/python3.5/encodings/cp437.py",
    "lib/python3.5/encodings/latin_1.py",
    "lib/python3.5/encodings/utf_8.py",
    "lib/python3.5/functools.py",
    "lib/python3.5/genericpath.py",
    "lib/python3.5/heapq.py",
    "lib/python3.5/importlib/__init__.py",
    "lib/python3.5/importlib/_bootstrap.py",
    "lib/python3.5/importlib/_bootstrap_external.py",
    "lib/python3.5/importlib/abc.py",
    "lib/python3.5/importlib/machinery.py",
    "lib/python3.5/importlib/util.py",
    "lib/python3.5/io.py",
    "lib/python3.5/keyword.py",
    "lib/python3.5/operator.py",
    "lib/python3.5/os.py",
    "lib/python3.5/pkgutil.py",
    "lib/python3.5/posixpath.py",
    "lib/python3.5/reprlib.py",
    "lib/python3.5/runpy.py",
    "lib/python3.5/site.py",
    "lib/python3.5/stat.py",
    "lib/python3.5/sysconfig.py",
    "lib/python3.5/types.py",
    "lib/python3.5/warnings.py",
    "lib/python3.5/weakref.py",
]


@task
def build_emscripten_cpython(ctx):
    check_correct_emscripten(EMSCRIPTEN_DIR)
    make_dir = join(PROJ_ROOT, "cpython-emscripten", "3.5.2")
    call("make", cwd=make_dir, shell=True)


@task
def build_pyodide_cpython(ctx):
    check_correct_emscripten(PY_EMSCRIPTEN_DIR)

    cpython_root = join(PYODIDE_ROOT, "cpython")

    # Remove lib to force make to run
    py_lib = join(cpython_root, "installs/python-3.7.0/lib/libpython3.7.a")
    call("rm -f {}".format(py_lib), shell=True)

    call("make", cwd=cpython_root, shell=True)


@task
def set_up_python_root(ctx):
    # Create root if necessary
    if not exists(FAASM_RUNTIME_ROOT):
        raise RuntimeError("Must create runtime root at {}".format(FAASM_RUNTIME_ROOT))

    # Clear out the destination
    call("rm -rf {}/*".format(FAASM_RUNTIME_ROOT), shell=True)

    # Iterate through and put files in place
    for relative_path in EMSCRIPTEN_PYTHON_LIB_FILES:
        # Create the intermediate directory
        file_name_parts = relative_path.split("/")
        relative_dir = "/".join(file_name_parts[:-1])
        call("mkdir -p {}/{}".format(FAASM_RUNTIME_ROOT, relative_dir), shell=True)

        dest_file = join(FAASM_RUNTIME_ROOT, relative_path)
        abs_path = "{}/{}".format(EMSCRIPTEN_CPYTHON_INSTALL_DIR, relative_path)

        cmd = "cp {} {}".format(abs_path, dest_file)
        print(cmd)
        res = call(cmd, shell=True)
        if res != 0:
            raise RuntimeError("Failed to put file {} in place".format(abs_path))


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

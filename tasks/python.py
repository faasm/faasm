import copy
from copy import copy
from os.path import join
from subprocess import call

from invoke import task

from tasks.compile import check_correct_emscripten
from tasks.download import FAASM_HOME, clone_proj
from tasks.env import NATIVE_ENV_DICT, ENV_STR, CONFIG_TARGET, \
    CONFIG_HOST, SYSROOT, PY_EMSCRIPTEN_DIR, PROJ_ROOT, PYODIDE_ROOT


@task
def python_codegen(ctx):
    codegen_bin = join(PROJ_ROOT, "cmake-build-debug", "bin", "codegen")

    with open(join(PROJ_ROOT, "python", "python_shared_obj.txt")) as fh:
        for so_file in fh:
            res = call("{} {}".format(codegen_bin, so_file), shell=True)
            if res != 0:
                print("Failed to generate machine code for {}".format(so_file))


@task
def build_pyodide_cpython(ctx):
    check_correct_emscripten(PY_EMSCRIPTEN_DIR)

    cpython_root = join(PYODIDE_ROOT, "cpython")

    # Remove lib to force make to run
    py_lib = join(cpython_root, "installs/python-3.7.0/lib/libpython3.7.a")
    call("rm -f {}".format(py_lib), shell=True)

    call("make", cwd=cpython_root, shell=True)


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

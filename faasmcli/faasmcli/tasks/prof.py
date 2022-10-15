from invoke import task
from faasmcli.util.env import PROJ_ROOT
from faasmcli.util.shell import find_command
from faasmcli.util.disassemble import replace_symbols_in_file
from os import makedirs
from os.path import join, exists
from shutil import rmtree
from subprocess import run

WORK_DIR = join(PROJ_ROOT, "dev")
FLAME_GRAPH_DIR = join(WORK_DIR, "FlameGraph")


@task()
def build_llvm_perf(ctx, clean=False):
    """
    Build LLVM with profiling enabled
    """
    # Upgrading the LLVM version here may break integration with WebAssembly
    # runtimes
    llvm_tag = "llvmorg-9.0.1"
    llvm_dir = join(WORK_DIR, "llvm-perf")

    if clean:
        rmtree(llvm_dir, ignore_errors=True)

    if not exists(llvm_dir):
        makedirs(llvm_dir)
        git_cmd = [
            "git clone",
            "--branch {}".format(llvm_tag),
            "--depth 1",
            "https://github.com/llvm/llvm-project",
            llvm_dir,
        ]
        git_cmd = " ".join(git_cmd)
        run(git_cmd, shell=True, check=True)

    build_dir = join(llvm_dir, "build")
    if not exists(build_dir):
        makedirs(build_dir)

    # TODO: pin compiler versions to variable
    cmake_cmd = [
        "cmake -G Ninja",
        "-DCMAKE_C_COMPILER=/usr/bin/clang-13",
        "-DCMAKE_CXX_COMPILER=/usr/bin/clang++-13",
        "-DCMAKE_BUILD_TYPE=RelWithDebInfo",
        "-DLLVM_USE_PERF=1",
        "-DLLVM_USE_INTEL_JITEVENTS=1",
        "-DLLVM_TARGETS_TO_BUILD=X86",
        "-DLLVM_INCLUDE_TOOLS=0",
        "-DLLVM_INCLUDE_TESTS=0",
        "-DLLVM_INCLUDE_EXAMPLES=0",
        "-DLLVM_INCLUDE_BENCHMARKS=0",
        "../llvm",
    ]
    cmake_cmd = " ".join(cmake_cmd)
    run(cmake_cmd, shell=True, check=True, cwd=build_dir)
    run("ninja", shell=True, check=True, cwd=build_dir)


@task()
def flame(ctx, user, func, cmd=None, data=None, reverse=False):
    """
    Generates a flame graph for the given function
    """
    print("Generating flame graph for {}/{}".format(user, func))

    if not exists(FLAME_GRAPH_DIR):
        print("Cloning FlameGraph")
        run(
            "git clone https://github.com/brendangregg/FlameGraph",
            cwd=WORK_DIR,
            shell=True,
            check=True,
        )

    # Set up the command to be perf'd
    if not cmd:
        func_runner_bin = find_command("func_runner")
        cmd = " ".join([func_runner_bin, user, func, data if data else ""])

    # Set up main perf command
    perf_cmd = ["perf", "record", "-k 1", "-F 99", "-g", cmd]
    perf_cmd = " ".join(perf_cmd)

    # Create list of commands to be run
    svg_file = join(PROJ_ROOT, "flame.svg")
    cmds = [
        perf_cmd,
        "perf inject -i perf.data -j -o perf.data.jit",
        "perf script -i perf.data.jit > out.perf",
        "./stackcollapse-perf.pl out.perf > out.folded",
        "./flamegraph.pl {} out.folded > {}".format(
            "--reverse" if reverse else "", svg_file
        ),
    ]

    # Execute each one in the flame graphs checkout
    for cmd in cmds:
        print(cmd)
        run(cmd, shell=True, check=True, cwd=FLAME_GRAPH_DIR)

    # Replace symbols in the SVG file
    replace_symbols_in_file(user, func, svg_file, prefix="wasm")

    print("\nFlame graph written to {}".format(svg_file))

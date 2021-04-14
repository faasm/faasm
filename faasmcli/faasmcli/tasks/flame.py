from os.path import join, exists

from invoke import task
from subprocess import run

from faasmcli.tasks.disas import replace
from faasmcli.util.env import WASM_DIR, PROJ_ROOT
from faasmcli.util.shell import run_command, find_command

WORK_DIR = join(PROJ_ROOT, "dev")
FLAME_GRAPH_DIR = join(WORK_DIR, "FlameGraph")


@task(default=True)
def full_run(ctx, user, func, reps=10):
    """
    Generates a flame graph for the given function
    """

    print(
        "Generating flame graph for {} reps of {}/{}".format(reps, user, func)
    )

    if not exists(FLAME_GRAPH_DIR):
        print("Cloning FlameGraph")
        run(
            "git clone https://github.com/brendangregg/FlameGraph",
            cwd=WORK_DIR,
            shell=True,
            check=True,
        )

    func_sym_bin = find_command("func_sym")
    syms_path = join(WASM_DIR, user, func, "function.symbols")

    # First generate the symbols for this function
    run("{} {} {}".format(func_sym_bin, user, func), shell=True, check=True)
    if not exists(syms_path):
        raise RuntimeError("Did not find symbols at {}".format(syms_path))

    # Set up main perf command
    simple_runner_bin = find_command("simple_runner")
    perf_cmd = [
        "perf",
        "record",
        "-k 1",
        "-F 99",
        "-g",
        simple_runner_bin,
        user,
        func,
        str(reps),
    ]

    perf_cmd = " ".join(perf_cmd)

    # Create list of commands to be run
    svg_file = join(PROJ_ROOT, "flame.svg")
    cmds = [
        perf_cmd,
        "perf inject -i perf.data -j -o perf.data.jit",
        "perf script -i perf.data.jit > out.perf",
        "./stackcollapse-perf.pl out.perf > out.folded",
        "./flamegraph.pl out.folded > {}".format(svg_file),
    ]

    # Execute each one in the flame graphs checkout
    for cmd in cmds:
        print(cmd)
        run(cmd, shell=True, check=True, cwd=FLAME_GRAPH_DIR)

    # Substitute all the symbols in the SVG file
    replace(ctx, user, func, svg_file)

    print("\nFlame graph written to {}".format(svg_file))

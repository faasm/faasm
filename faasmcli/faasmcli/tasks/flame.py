from os.path import join, exists

from invoke import task
from subprocess import run

from faasmcli.util.env import PROJ_ROOT
from faasmcli.util.shell import find_command
from faasmcli.util.disassemble import replace_symbols_in_file

WORK_DIR = join(PROJ_ROOT, "dev")
FLAME_GRAPH_DIR = join(WORK_DIR, "FlameGraph")


@task(default=True)
def general(ctx, user, func, reps=10, cmd=None, data=None, reverse=False):
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

    # Set up the command to be perf'd
    if not cmd:
        simple_runner_bin = find_command("simple_runner")
        cmd = " ".join(
            [simple_runner_bin, user, func, str(reps), data if data else ""]
        )

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

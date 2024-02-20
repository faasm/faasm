from invoke import task
from os.path import join
from subprocess import run
from tasks.util.env import LLVM_MAJOR_VERSION, PROJ_ROOT


@task(default=True)
def format(ctx, cwd=None, check=False):
    """
    Format Python and C++ code
    """
    if not cwd:
        cwd = PROJ_ROOT

    # ---- Python formatting ----

    files_to_check = (
        run(
            'git ls-files -- "*.py"',
            shell=True,
            check=True,
            cwd=cwd,
            capture_output=True,
        )
        .stdout.decode("utf-8")
        .split("\n")[:-1]
    )
    black_cmd = [
        "python3 -m black",
        "{}".format("--check" if check else ""),
        " ".join(files_to_check),
    ]
    black_cmd = " ".join(black_cmd)
    run(black_cmd, shell=True, check=True, cwd=cwd)

    flake8_cmd = [
        "python3 -m flake8",
        " ".join(files_to_check),
    ]
    flake8_cmd = " ".join(flake8_cmd)
    run(flake8_cmd, shell=True, check=True, cwd=cwd)

    # ---- C/C++ formatting ----

    files_to_check = (
        run(
            'git ls-files -- "*.h" "*.cpp" "*.c"',
            shell=True,
            check=True,
            cwd=cwd,
            capture_output=True,
        )
        .stdout.decode("utf-8")
        .split("\n")[:-1]
    )

    clang_cmd = [
        "clang-format-{}".format(LLVM_MAJOR_VERSION),
        "--dry-run --Werror" if check else "-i",
        " ".join(files_to_check),
    ]
    clang_cmd = " ".join(clang_cmd)
    run(clang_cmd, shell=True, check=True, cwd=cwd)

    # ---- Append newlines to C/C++ files if not there ----

    for f in files_to_check:
        # Append opens the file from the end, but there is no easy way to read
        # just one character backwards unless you open the file as a byte
        # stream, which then makes it very involved to compare against the
        # newline character
        with open(join(cwd, f), "a+") as fh:
            fh.seek(0)
            read = fh.read()
            if read[-1] != "\n":
                fh.write("\n")

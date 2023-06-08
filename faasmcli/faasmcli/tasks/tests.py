from invoke import task
from os import listdir
from os.path import join
from subprocess import run
from faasmcli.util.env import (
    FAASM_BUILD_DIR,
    PROJ_ROOT,
)

TEST_ENV = {
    "CGROUP_MODE": "on",
    "LOG_LEVEL": "info",
    "NETNS_MODE": "off",
    "PLANNER_HOST": "planner",
    "REDIS_QUEUE_HOST": "redis",
    "REDIS_STATE_HOST": "redis",
    "TERM": "xterm-256color",
    "WASM_VM": "wavm",
    # Sanitiser env. variables
    "ASAN_OPTIONS": "halt_on_error=1:quarantine_size_mb=16",
    "LSAN_OPTIONS": "suppressions={}/leak-sanitizer-ignorelist.txt".format(PROJ_ROOT),
    "TSAN_OPTIONS": " ".join(
        [
            "history_size=0",
            "halt_on_error=1",
            "suppressions={}/thread-sanitizer-ignorelist.txt".format(
                PROJ_ROOT
            ),
            "flush_memory_ms=5000",
        ]
    ),
    "UBSAN_OPTIONS": ":".join(
        [
            "print_stacktrace=1",
            "halt_on_error=1",
            "suppressions={}/ub-sanitizer-ignorelist.txt",
        ]
    ),
}


@task(default=True)
def tests(
    ctx,
    test_case=None,
    test_file=None,
    test_dir=None,
    abort=False,
    debug=False,
    repeats=1,
):
    """
    Run the C++ unit tests

    When running this task with no arguments, the whole test suite will be
    executed. You can specify the name of the test to run by passing the
    --test-case variable. Additionally, you may also specify a filename to
    run (--filename) or a directory (--directory)
    """
    tests_cmd = [
        join(FAASM_BUILD_DIR, "bin", "tests"),
        "--use-colour yes",
        "--abort" if abort else "",
    ]

    if debug:
        TEST_ENV["LOG_LEVEL"] = "debug"

    if test_case:
        tests_cmd.append("'{}'".format(test_case))
    elif test_file:
        tests_cmd.append("--filenames-as-tags [#{}]".format(test_file))
    elif test_dir:
        tag_str = "--filenames-as-tags "
        for file_name in listdir(join(PROJ_ROOT, "tests", "test", test_dir)):
            tag_str += "[#{}],".format(file_name.split(".")[0])
        tests_cmd.append(tag_str[:-1])

    tests_cmd = " ".join(tests_cmd)
    for i in range(repeats):
        run(tests_cmd, shell=True, check=True, cwd=PROJ_ROOT, env=TEST_ENV)

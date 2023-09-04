from invoke import task
from os import environ, listdir
from os.path import join
from subprocess import run
from faasmcli.util.env import (
    FAASM_BUILD_DIR,
    PROJ_ROOT,
)

# Depending on whether we run the tests on GHA or locally, some env. variables
# related to service names, or ports, need to change
IS_CI = "HOST_TYPE" in environ and environ["HOST_TYPE"] == "ci"

TEST_ENV = {
    "CGROUP_MODE": "off" if IS_CI else "on",
    "LD_LIBRARY_PATH": "/build/faasm/third-party/lib:/usr/local/lib",
    "LOG_LEVEL": "info",
    "NETNS_MODE": "off",
    "PLANNER_HOST": "localhost",
    "PLANNER_PORT": "8080" if IS_CI else "8081",
    "REDIS_QUEUE_HOST": "redis" if IS_CI else "redis-queue",
    "REDIS_STATE_HOST": "redis" if IS_CI else "redis-state",
    "TERM": "xterm-256color",
    "WASM_VM": "wavm",
    # Sanitiser env. variables
    "ASAN_OPTIONS": "halt_on_error=1:quarantine_size_mb=16",
    "LSAN_OPTIONS": "suppressions={}/leak-sanitizer-ignorelist.txt".format(
        PROJ_ROOT
    ),
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
            "suppressions={}/ub-sanitizer-ignorelist.txt".format(PROJ_ROOT),
        ]
    ),
}

# If executing in CI, give us some extra CPU cores to run the tests
if IS_CI:
    TEST_ENV["OVERRIDE_CPU_COUNT"] = "10"

ENV_VAR_ALLOWLIST = ["HOST_TYPE", "LLVM_PROFILE_FILE"]


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

    # Allow specific environment variables
    for env_var in ENV_VAR_ALLOWLIST:
        if env_var in environ:
            TEST_ENV[env_var] = environ[env_var]

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

    # TODO: run ./bin/cgroup.sh ?

    tests_cmd = " ".join(tests_cmd)
    for i in range(repeats):
        run(tests_cmd, shell=True, check=True, cwd=PROJ_ROOT, env=TEST_ENV)

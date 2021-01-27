import os


def _check_env_var(name, expected):
    actual = os.environ.get(name)

    if actual != expected:
        print("{} unexpected: {} != {}".format(name, actual, expected))
        return False

    print("{}={}".format(name, actual))
    return True


def faasm_main():
    success = True

    success &= _check_env_var("PYTHON_LOCAL_CHAINING", None)
    success &= _check_env_var("PYTHON_LOCAL_OUTPUT", None)

    success &= _check_env_var("PYTHONWASM", "1")

    success &= _check_env_var("FOOBAR", None)

    return 0 if success else 1

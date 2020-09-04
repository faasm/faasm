from os.path import dirname, realpath, join

_PROJ_ROOT = dirname(realpath(__file__))


def Settings(**kwargs):
    venv_interpreter = join(_PROJ_ROOT, "venv", "bin", "python")
    return {"interpreter_path": venv_interpreter}

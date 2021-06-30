from faasmcli.util.shell import find_command


def find_codegen_shared_lib():
    return find_command("codegen_shared_obj")


def find_codegen_func():
    return find_command("codegen_func")

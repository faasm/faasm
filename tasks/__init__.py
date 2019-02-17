from tasks.compile import (
    build_python,
    compile,
    clean_build,
    funcs,
    lib,
)

from tasks.docker import (
    build_all,
    build_base,
    build_data,
    build_edge,
    build_redis,
    build_worker,
    build_upload,
    build_toolchain,
    data,
    purge_images,
    push_all,
    push_data,
    push_edge,
    push_redis,
    push_upload,
    push_worker,
    push_toolchain,
    tools,
    pull,
    push_base
)

from tasks.docker_compose import start_all, stop_all, restart_all

from tasks.aws import (
    build_lambda_runtime,
    build_lambdas,
    upload_lambda_function,
)

from tasks.upload import upload

from tasks.wasm2wast import wast


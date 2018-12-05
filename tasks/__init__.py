from tasks.compile import lib, compile, funcs

from tasks.docker import (
    build_base,
    build_edge,
    build_worker,
    build_upload,
    build_toolchain,
    push_edge,
    push_upload,
    push_worker,
    push_toolchain,
    tools,
    pull,
    push_base
)

from tasks.docker_compose import start_all, stop_all, restart_all

from tasks.reuters import reuters_download

from tasks.setup import (
    setup_namespaces,
    setup_net_files,
    setup_libs,
    setup_clang,
    setup_eigen,
    setup_cgroup,
    setup_protobuf,
    setup_spdlog,
    setup_pistache
)

from tasks.state import upload_sparse_matrix

from tasks.upload import upload

from tasks.wasm2wast import wast


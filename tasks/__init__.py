from tasks.compile import lib, compile
from tasks.docker import build_base, build_edge, build_worker, build_toolchain, push_edge, push_worker, push_toolchain, \
    tools
from tasks.docker_compose import start_all, stop_all, restart_all
from tasks.setup import setup_namespaces, setup_net_files, setup_libs, setup_clang, setup_cgroup, setup_protobuf
from tasks.upload import upload
from tasks.wasm2wast import wast

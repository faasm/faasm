from invoke import Collection

from . import call
from . import cluster
from . import codegen
from . import config
from . import dev
from . import disas
from . import docker_tasks
from . import docs
from . import files
from . import flame
from . import flush
from . import format_code
from . import git
from . import k8s
from . import network
from . import python
from . import redis
from . import run
from . import sgx
from . import state
from . import upload
from . import wast

# Default names
ns = Collection(
    cluster,
    codegen,
    config,
    dev,
    disas,
    docs,
    files,
    flame,
    flush,
    format_code,
    git,
    k8s,
    network,
    python,
    redis,
    run,
    sgx,
    state,
    upload,
    wast,
)

# Custom names
ns.add_collection(ns.from_module(call), name="invoke")
ns.add_collection(ns.from_module(docker_tasks), name="docker")

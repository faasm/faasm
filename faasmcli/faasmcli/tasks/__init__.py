from invoke import Collection

from . import bare_metal
from . import call
from . import codegen
from . import compile
from . import config
from . import dev
from . import disas
from . import docker_tasks
from . import github
from . import knative
from . import libs
from . import python
from . import redis
from . import run
from . import state
from . import toolchain
from . import upload
from . import wast

# Default names
ns = Collection(
    codegen,
    compile,
    config,
    dev,
    disas,
    github,
    knative,
    libs,
    python,
    redis,
    run,
    state,
    toolchain,
    upload,
    wast,
)

# Custom names
ns.add_collection(ns.from_module(bare_metal), name="bm")
ns.add_collection(ns.from_module(call), name="invoke")
ns.add_collection(ns.from_module(docker_tasks), name="docker")

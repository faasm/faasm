from invoke import Collection

from . import bare_metal
from . import call
from . import codegen
from . import config
from . import dev
from . import disas
from . import docker_tasks
from . import flame
from . import flush
from . import knative
from . import python
from . import redis
from . import release
from . import run
from . import state
from . import upload
from . import wast

# Default names
ns = Collection(
    codegen,
    config,
    dev,
    disas,
    flame,
    flush,
    knative,
    python,
    redis,
    release,
    run,
    state,
    upload,
    wast,
)

# Custom names
ns.add_collection(ns.from_module(bare_metal), name="bm")
ns.add_collection(ns.from_module(call), name="invoke")
ns.add_collection(ns.from_module(docker_tasks), name="docker")

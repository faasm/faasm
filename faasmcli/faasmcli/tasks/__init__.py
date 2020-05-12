from invoke import Collection

# from . import ibm
from . import bare_metal
from . import call
from . import codegen
from . import compile
from . import config
from . import disas
from . import docker_tasks
from . import github
from . import knative
from . import libs
from . import prk
from . import python
from . import redis
from . import run
from . import state
from . import tensorflow
from . import toolchain
from . import upload
from . import wast

# Default names
ns = Collection(
    # ibm,
    codegen,
    compile,
    config,
    disas,
    github,
    knative,
    libs,
    prk,
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
ns.add_collection(ns.from_module(tensorflow), name="tf")

# Can only generate matrix data with things installed
try:
    import pyfaasm
    from . import matrix_data

    ns.add_collection(ns.from_module(matrix_data))
except:
    pass

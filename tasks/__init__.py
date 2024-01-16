from invoke import Collection

from . import codegen
from . import dev
from . import disas
from . import docker
from . import docs
from . import flame
from . import format_code
from . import git
from . import network
from . import python
from . import run
from . import sgx
from . import tests
from . import wast

# Default names
ns = Collection(
    codegen,
    dev,
    disas,
    docker,
    docs,
    flame,
    format_code,
    git,
    network,
    python,
    run,
    sgx,
    tests,
    wast,
)

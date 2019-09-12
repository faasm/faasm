from os.path import join

from tasks.util.env import PROJ_ROOT

TS_DIR = join(PROJ_ROOT, "typescript")
ASC_BINARY = join(TS_DIR, "node_modules", "assemblyscript", "bin", "asc")

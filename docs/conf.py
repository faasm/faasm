from os.path import dirname, realpath, join
from subprocess import run
from os import makedirs

# ----------------------------------------
# This is the Sphinx configuration file, which wraps
# the calls to Doxygen and Breathe.
# ----------------------------------------

DOCS_ROOT = dirname(realpath(__file__))
DOXYGEN_OUT = join(DOCS_ROOT, "doxygen", "xml")
APIDOC_OUT_DIR = join(DOCS_ROOT, "apidoc")

makedirs(APIDOC_OUT_DIR, exist_ok=True)

run("doxygen", cwd=DOCS_ROOT, check=True, shell=True)

run(
    "breathe-apidoc {} -o {} -f -m".format(
        DOXYGEN_OUT, APIDOC_OUT_DIR
    ),
    cwd=DOCS_ROOT,
    check=True,
    shell=True,
)

project = "Faasm"
copyright = "2022, Simon Shillaker"
author = "Simon Shillaker"

extensions = ["breathe", "myst_parser", "sphinx_rtd_theme"]

templates_path = ["source/templates"]
html_static_path = ["source/static"]

exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

html_theme = "sphinx_rtd_theme"

breathe_projects = {"Faasm": DOXYGEN_OUT}
breathe_default_project = "Faasm"
breathe_default_members = ("members", "undoc-members")


from faasmtools.docker import CR_NAME
from github import Github
from invoke import task
from os import environ
from os.path import exists, join
from re import escape
from shlex import quote
from subprocess import run, PIPE, STDOUT
from tasks.util.env import PROJ_ROOT
from tasks.util.version import get_version

REPO_NAME = "faasm/faasm"

VERSIONED_FILES = {
    "faasm": [".env", "VERSION"],
    "faabric": [
        ".env",
        ".github/workflows/tests.yml",
        "./deploy/k8s-common/planner.yml",
    ],
    "cpp": [".env", ".github/workflows/tests.yml"],
    "python": [".env", ".github/workflows/tests.yml"],
    "faasmctl": [
        "requirements.txt",
        ".github/workflows/tests.yml",
        ".github/workflows/azure.yml",
        ".github/workflows/sgx_hw.yml",
    ],
}

VERSIONED_DIRS = [
    join(PROJ_ROOT, ".github"),
    join(PROJ_ROOT, "deploy"),
]


def _tag_name(version):
    return "v{}".format(version)


def _get_tag():
    faasm_ver = get_version()
    tag_name = _tag_name(faasm_ver)
    return tag_name


def _get_current_branch_name():
    branch_out = run(
        "git rev-parse --abbrev-ref HEAD",
        shell=True,
        stdout=PIPE,
        stderr=STDOUT,
    )

    branch_name = branch_out.stdout.decode()
    branch_name = branch_name.strip()
    return branch_name


def _get_release():
    r = _get_repo()
    rels = r.get_releases()

    return rels[0]


def _get_github_instance():
    if "GITHUB_TOKEN" in environ:
        token = environ["GITHUB_TOKEN"]
    else:
        filename = join(PROJ_ROOT, "dev", "GITHUB_TOKEN")

        if exists(filename):
            with open(filename, "r") as fh:
                token = fh.read()
                token = token.strip()
        else:
            print("Must set GITHUB_TOKEN before creating a release")
            raise RuntimeError(
                "Must set GITHUB_TOKEN before creating a release"
            )

    g = Github(token)
    return g


def _get_repo():
    g = _get_github_instance()
    return g.get_repo(REPO_NAME)


def _create_tag(tag_name, force=False):
    # Create the tag
    run(
        "git tag {} {}".format("--force" if force else "", tag_name),
        shell=True,
        check=True,
        cwd=PROJ_ROOT,
    )

    # Push tag
    run(
        "git push {} origin {}".format("--force" if force else "", tag_name),
        shell=True,
        check=True,
        cwd=PROJ_ROOT,
    )


@task
def bump(ctx, patch=False, minor=False, major=False):
    """
    Bump the code version by: --patch, --minor, or --major

    Faasm uses SemVer to tag code versions. For more information, see the
    website: https://semver.org/
    """
    old_ver = get_version()
    new_ver_parts = old_ver.split(".")

    if patch:
        idx = 2
    elif minor:
        idx = 1
    elif major:
        idx = 0
    else:
        raise RuntimeError("Must set one in: --[patch,minor,major]")

    # Change the corresponding idx
    new_ver_parts[idx] = str(int(new_ver_parts[idx]) + 1)

    # Zero-out the following version numbers (i.e. lower priority). This is
    # because if we tag a new major release, we want to zero-out the minor
    # and patch versions (e.g. 0.2.0 comes after 0.1.9)
    for next_idx in range(idx + 1, 3):
        new_ver_parts[next_idx] = "0"

    new_ver = ".".join(new_ver_parts)

    # Replace version in all files
    for f in VERSIONED_FILES["faasm"]:
        sed_cmd = "sed -i 's/{}/{}/g' {}".format(old_ver, new_ver, f)
        run(sed_cmd, shell=True, check=True)

    # Replace version in dirs (only for faasm)
    for d in VERSIONED_DIRS:
        sed_cmd = [
            "find {}".format(d),
            "-type f",
            "-exec sed -i -e 's/{}/{}/g'".format(old_ver, new_ver),
            "{} \\;",
        ]
        sed_cmd = " ".join(sed_cmd)
        print(sed_cmd)

        run(sed_cmd, shell=True, check=True)


@task
def bump_dep(ctx, faasmctl=None, python=False, cpp=False, faabric=False):
    """
    Bump the version of a project dep.: faasmctl, cpp, python, or faabric

    Faasm has four tightly coupled dependencies. Three of them: faabric, cpp,
    and python are tracked as submodules. Thus, "bump"-ing the version means
    making sure that the version we track in Faasm is aligned with that in the
    submodule. For the latest dependency, `faasmctl` we install it as a PIP
    dependency (but it changes often) so we may want to bump to an arbitrary
    version
    """
    # The python and cpp versions might be the same, so we need to be more
    # careful when we increment each of them to make sure we only increment
    # the version of the client we are interested in
    if python:
        old_ver, new_ver = get_version("python")
        strings_to_check = [
            join(CR_NAME, "cpython:"),
            "PYTHON_VERSION=",
        ]
        for f in VERSIONED_FILES["python"]:
            for string in strings_to_check:
                # Pattern must be escaped for sed.
                pattern = escape(f"{string}{old_ver}")
                replacement = f"{string}{new_ver}"
                sed_cmd = f"sed -i 's|{pattern}|{replacement}|g' {quote(f)}"
                print(sed_cmd)
                run(sed_cmd, shell=True, check=True)

    if cpp:
        old_ver, new_ver = get_version("cpp")
        strings_to_check = [
            join(CR_NAME, "cpp-sysroot:"),
            "CPP_VERSION=",
        ]
        for f in VERSIONED_FILES["cpp"]:
            for string in strings_to_check:
                # Pattern must be escaped for sed.
                pattern = escape(f"{string}{old_ver}")
                replacement = f"{string}{new_ver}"
                sed_cmd = f"sed -i 's|{pattern}|{replacement}|g' {quote(f)}"
                print(sed_cmd)
                run(sed_cmd, shell=True, check=True)

    if faabric:
        old_ver, new_ver = get_version("faabric")
        strings_to_check = [
            join(CR_NAME, "planner:"),
            "FAABRIC_VERSION=",
            "FAABRIC_VERSION: ",
        ]
        for f in VERSIONED_FILES["faabric"]:
            for string in strings_to_check:
                # Pattern must be escaped for sed.
                pattern = escape(f"{string}{old_ver}")
                replacement = f"{string}{new_ver}"
                sed_cmd = f"sed -i 's|{pattern}|{replacement}|g' {quote(f)}"
                print(sed_cmd)
                run(sed_cmd, shell=True, check=True)

    if faasmctl is not None:
        new_ver = faasmctl
        old_ver = get_version("faasmctl")
        strings_to_check = ["faasmctl==", "FAASMCTL_VERSION: "]
        for f in VERSIONED_FILES["faasmctl"]:
            for string in strings_to_check:
                # Pattern must be escaped for sed.
                pattern = escape(f"{string}{old_ver}")
                replacement = f"{string}{new_ver}"
                sed_cmd = f"sed -i 's|{pattern}|{replacement}|g' {quote(f)}"
                print(sed_cmd)
                run(sed_cmd, shell=True, check=True)


@task
def tag(ctx, force=False):
    """
    Tags the latest commit on the current branch
    """
    # Work out the tag name
    tag_name = _get_tag()
    branch_name = _get_current_branch_name()

    # Create a tag from the current branch
    print(
        "Creating tag {} from current branch {}".format(tag_name, branch_name)
    )

    _create_tag(tag_name, force=force)


def get_release_body():
    """
    Generate body for release with detailed changelog
    """
    git_cmd = (
        "git log --pretty=format:'%d,%s,%as' {}...v{}".format(
            _get_release().tag_name, get_version()
        ),
    )
    commits = (
        run(git_cmd, shell=True, capture_output=True, cwd=PROJ_ROOT)
        .stdout.decode("utf-8")
        .split("\n")
    )
    body = "Here is what has changed since last release:\n"

    def make_tag_header(body, tag, date):
        tag = tag.split(" ")[2][:-1]
        body += "\n## [{}] - {}\n".format(tag, date)
        return body

    def get_commit_parts(commit):
        first_comma = commit.find(",")
        last_comma = commit.rfind(",")
        tag_end = first_comma
        msg_start = first_comma + 1
        msg_end = last_comma
        date_start = last_comma + 1
        tag = commit[0:tag_end]
        msg = commit[msg_start:msg_end]
        date = commit[date_start:]
        return tag, msg, date

    for commit in commits:
        tag, msg, date = get_commit_parts(commit)
        if tag:
            body = make_tag_header(body, tag, date)
        body += "* {}\n".format(msg)
    return body.strip()


@task
def create_release(ctx):
    """
    Create a draft release on Github
    """
    # Work out the tag
    faasm_ver = get_version()
    tag_name = _tag_name(faasm_ver)

    # Create a release in github from this tag
    r = _get_repo()
    r.create_git_release(
        tag_name,
        "Faasm {}".format(faasm_ver),
        get_release_body(),
        draft=True,
    )


@task
def publish_release(ctx):
    """
    Publish the draft release
    """
    rel = _get_release()
    rel.update_release(rel.title, rel.raw_data["body"], draft=False)


@task
def check_submodule_branch(ctx):
    """
    Check that the commit for each submodule is a commit in the main branch
    """
    submodules = ["faabric", "clients/cpp", "clients/python"]
    # First, work out what commit each submodule points to
    git_cmd = "git submodule status"
    sub_status = (
        run(git_cmd, shell=True, capture_output=True, cwd=PROJ_ROOT)
        .stdout.decode("utf-8")
        .split("\n")
    )

    # Then, for each submodule, check if the pointed-to commit is an ancestor
    # of the commit tagged with `main`
    for submodule in submodules:
        pointed_to_commit = [
            line.split(" ") for line in sub_status if submodule in line
        ][0]
        pointed_to_commit = [word for word in pointed_to_commit if word][0]
        print(submodule, pointed_to_commit)

        git_ancestor_cmd = "git merge-base --is-ancestor {} main".format(
            pointed_to_commit
        )
        ret_code = run(
            git_ancestor_cmd, shell=True, cwd=join(PROJ_ROOT, submodule)
        ).returncode
        if ret_code != 0:
            print(
                "ERROR: submodule {} points to a dangling commit ({})".format(
                    submodule, pointed_to_commit
                )
            )
            raise RuntimeError("Submodule pointing to dangling commit")

from faasmcli.util.env import PROJ_ROOT
from faasmcli.util.config import get_faasm_config
from faasmcli.util.version import get_version
from faasmtools.docker import ACR_NAME
from github import Github
from invoke import task
from os.path import join
from subprocess import run, PIPE, STDOUT

REPO_NAME = "faasm/faasm"

VERSIONED_FILES = {
    "faasm": [".env", "VERSION"],
    "faabric": [".env", ".github/workflows/tests.yml", "deploy/k8s-common/planner.yml"],
    "cpp": [".env", ".github/workflows/tests.yml"],
    "python": [".env", ".github/workflows/tests.yml"],
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
    conf = get_faasm_config()

    if not conf.has_section("Github") or not conf.has_option(
        "Github", "access_token"
    ):
        print("Must set up Github config with access token")

    token = conf["Github"]["access_token"]
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
def bump(ctx, ver=None, python=False, cpp=False, faabric=False):
    """
    Increase the version (defaults to bumping a single minor version)
    """
    bump_faasm_ver = (not python) and (not cpp) and (not faabric)
    if bump_faasm_ver:
        old_ver = get_version()
        if ver:
            new_ver = ver
        else:
            # Just bump the last minor version part
            new_ver_parts = old_ver.split(".")
            new_ver_minor = int(new_ver_parts[-1]) + 1
            new_ver_parts[-1] = str(new_ver_minor)
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
    else:
        # The python and cpp versions might be the same, so we need to be more
        # careful when we increment each of them to make sure we only increment
        # the version of the client we are interested in
        if python:
            old_ver, new_ver = get_version("python")
            strings_to_check = [
                r"{}\/cpython:".format(ACR_NAME),
                "PYTHON_VERSION=",
            ]
            for f in VERSIONED_FILES["python"]:
                for string in strings_to_check:
                    sed_cmd = "sed -i 's/{}{}/{}{}/g' {}".format(
                        string, old_ver, string, new_ver, f
                    )
                    print(sed_cmd)
                    run(sed_cmd, shell=True, check=True)
        if cpp:
            old_ver, new_ver = get_version("cpp")
            strings_to_check = [
                r"{}\/cpp-sysroot:".format(ACR_NAME),
                "CPP_VERSION=",
            ]
            for f in VERSIONED_FILES["python"]:
                for string in strings_to_check:
                    sed_cmd = "sed -i 's/{}{}/{}{}/g' {}".format(
                        string, old_ver, string, new_ver, f
                    )
                    print(sed_cmd)
                    run(sed_cmd, shell=True, check=True)
        if faabric:
            old_ver, new_ver = get_version("faabric")
            strings_to_check = [
                r"{}\/planner:".format(ACR_NAME),
                "FAABRIC_VERSION=",
            ]
            for f in VERSIONED_FILES["python"]:
                for string in strings_to_check:
                    sed_cmd = "sed -i 's/{}{}/{}{}/g' {}".format(
                        string, old_ver, string, new_ver, f
                    )
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

from github import Github
from invoke import task
from subprocess import run, PIPE, STDOUT
from os.path import join

from faasmcli.util.env import PROJ_ROOT
from faasmcli.util.config import get_faasm_config
from faasmcli.util.version import get_faasm_version

REPO_NAME = "faasm/faasm"

VERSIONED_FILES = [
    ".env",
    "VERSION",
]

VERSIONED_DIRS = [
    join(PROJ_ROOT, ".github"),
    join(PROJ_ROOT, "deploy"),
]


def _tag_name(version):
    return "v{}".format(version)


def _get_tag():
    faasm_ver = get_faasm_version()
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
def bump(ctx, ver=None):
    """
    Increase the version (defaults to bumping a single minor version)
    """
    old_ver = get_faasm_version()

    if ver:
        new_ver = ver
    else:
        # Just bump the last minor version part
        new_ver_parts = old_ver.split(".")
        new_ver_minor = int(new_ver_parts[-1]) + 1
        new_ver_parts[-1] = str(new_ver_minor)
        new_ver = ".".join(new_ver_parts)

    # Replace version in all files
    for f in VERSIONED_FILES:
        sed_cmd = "sed -i 's/{}/{}/g' {}".format(old_ver, new_ver, f)
        run(sed_cmd, shell=True, check=True)

    # Replace version in dirs
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
    docker_cmd = [
        "docker run -t -v",
        "{}:/app/".format(PROJ_ROOT),
        "orhunp/git-cliff:latest",
        "--config cliff.toml",
        "--repository .",
        "{}..v{}".format(_get_release().tag_name, get_faasm_version()),
    ]

    cmd = " ".join(docker_cmd)
    print("Generating release body...")
    print(cmd)
    result = run(cmd, shell=True, stdout=PIPE, stderr=PIPE)

    return result.stdout.decode("utf-8")


@task
def create_release(ctx):
    """
    Create a draft release on Github
    """
    # Work out the tag
    faasm_ver = get_faasm_version()
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

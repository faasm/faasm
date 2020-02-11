from github import Github
from invoke import task

from tasks.util.config import get_faasm_config
from tasks.util.version import get_faasm_version

REPO_NAME = "lsds/Faasm"


def _get_github_instance():
    conf = get_faasm_config()

    if not conf.has_section("Github") or not conf.has_option("Github", "access_token"):
        print("Must set up Github config with access token")

    token = conf["Github"]["access_token"]
    g = Github(token)
    return g


def _get_repo():
    g = _get_github_instance()
    return g.get_repo(REPO_NAME)


@task
def gh_print_repos(ctx):
    g = _get_github_instance()

    for repo in g.get_user().get_repos():
        print(repo.name)


@task
def gh_create_release(ctx):
    # Get the head of master
    r = _get_repo()
    b = r.get_branch(branch="master")
    head = b.commit

    # version = get_faasm_version()
    version = "0.0.5"

    # Create a tag from the head
    tag_name = "v{}".format(version)
    tag = r.create_git_tag(
        tag_name,
        "Release {}\n".format(version),
        head.sha,
        "commit",
    )

    r.create_git_release(
        tag_name,
        "Faasm {}".format(version),
        "Release {}\n".format(version),
        draft=True
    )


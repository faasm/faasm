from github import Github
from invoke import task

from tasks.util.config import get_faasm_config
from tasks.util.release import tar_toolchain, tar_sysroot, tar_runtime_root
from tasks.util.version import get_faasm_version

REPO_NAME = "lsds/Faasm"


def _tag_name(version):
    return "v{}".format(version)


def _get_release():
    version = get_faasm_version()
    r = _get_repo()
    rels = r.get_releases()
    tag_name = _tag_name(version)

    rel = rels[0]
    if rel.tag_name != tag_name:
        print("Expected latest release to have tag {} but had {}".format(tag_name, rel.tag_name))
        exit(1)

    return rel


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
def create_release(ctx):
    """
    Create a draft release on Github
    """
    # Get the head of master
    r = _get_repo()
    b = r.get_branch(branch="master")
    head = b.commit

    version = get_faasm_version()

    # Create a tag from the head
    tag_name = _tag_name(version)
    r.create_git_tag(
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


@task
def upload_artifacts(ctx):
    """
    Upload release artifacts
    """
    rel = _get_release()

    # Zip the relevant artifacts
    toolchain_name, toolchain_path = tar_toolchain()
    sysroot_name, sysroot_path = tar_sysroot()
    runtime_name, runtime_path = tar_runtime_root()

    # Upload assets
    print("Uploading toolchain to GH")
    rel.upload_asset(toolchain_path, label=toolchain_name)

    print("Uploading sysroot to GH")
    rel.upload_asset(sysroot_path, label=sysroot_name)

    print("Uploading runtime root to GH")
    rel.upload_asset(runtime_path, label=runtime_name)


@task
def publish_release(ctx):
    """
    Publish the draft release
    """
    rel = _get_release()
    rel.update_release(rel.title, rel.raw_data["body"], draft=False)

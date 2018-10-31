from invoke import task

from tasks.download import download_proj

URL = "https://archive.ics.uci.edu/ml/machine-learning-databases/00259/rcv1rcv2aminigoutte.tar.bz2"


@task
def reuters_download(ctx):
    extract_dir, build_dir = download_proj(
        URL,
        "rcv1rcv2aminigoutte",
        extension="tar.bz2"
    )

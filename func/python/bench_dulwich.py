import os
from os.path import exists

import dulwich.repo


def iter_all_commits(repo):
    # Iterate on all changes on the Git repository
    for entry in repo.get_walker(head):
        pass


if __name__ == "__main__":
    if os.environ.get("PYTHONWASM") == "1":
        repo_path = "/lib/python3.7/site-packages/pyperformance/benchmarks/data/asyncio.git"
    else:
        repo_path = "/usr/local/code/faasm/venv/lib/python3.6/site-packages/pyperformance/benchmarks/data/asyncio.git"
        if not exists(repo_path):
            repo_path = "/usr/local/lib/python3.6/dist-packages/pyperformance/benchmarks/data/asyncio.git"

    repo = dulwich.repo.Repo(repo_path)
    head = repo.head()

    iter_all_commits(repo)
    repo.close()

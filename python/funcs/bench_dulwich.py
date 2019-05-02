import dulwich.repo


def iter_all_commits(repo):
    # iterate on all changes on the Git repository
    for entry in repo.get_walker(head):
        pass


if __name__ == "__main__":
    # Note, this is a Faasm-specific path
    repo_path = "/lib/python3.7/site-packages/performance/benchmarks/data/asyncio.git"
    repo = dulwich.repo.Repo(repo_path)
    head = repo.head()

    iter_all_commits(repo)
    repo.close()

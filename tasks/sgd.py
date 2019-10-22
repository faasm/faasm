from os import mkdir
from os.path import exists, join
from shutil import rmtree

from invoke import task

from tasks.util.invoke import invoke_impl


@task
def sgd_experiment(ctx):
    workers = [2, 6, 10, 14, 18, 22, 26, 30, 34, 38]
    intervals = [600000, 60000]

    for w in workers:
        for i in intervals:
            _do_call(w, i)


def _do_call(n_workers, interval):
    success, output = invoke_impl(
        "sgd", "reuters_svm",
        poll=True, knative=True,
        input="{} {}".format(n_workers, interval)
    )

    if not success:
        print("FAILED on {} {}".format(n_workers, interval))

    output_dir = "/usr/local/code/serverless-problem/measurement/committed/sgd"
    if not exists(output_dir):
        mkdir(output_dir)

    folder_name = "SYSTEM_FAASM_WORKERS_{}_INTERVAL_{}_logs".format(n_workers, interval)
    result_dir = join(output_dir, folder_name)
    if exists(result_dir):
        rmtree(result_dir)
    mkdir(result_dir)

    output_file = join(result_dir, "NODE_0_SGD_LOSS.log")
    with open(output_file, "w") as fh:
        fh.write(output)

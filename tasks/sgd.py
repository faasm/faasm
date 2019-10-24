from os import mkdir
from os.path import exists, join
from shutil import rmtree
from subprocess import call

from invoke import task

from tasks.util.billing import start_billing, pull_billing
from tasks.util.env import FAASM_HOME
from tasks.util.invoke import invoke_impl


@task
def sgd_experiment(ctx, native=False):
    workers = [2, 6]
    intervals = [60000]

    for w in workers:
        for i in intervals:
            _do_call(w, i, native)


def _do_call(n_workers, interval, native):
    # Start the billing scripts
    start_billing()

    # Run the request
    success, output = invoke_impl(
        "sgd", "reuters_svm",
        poll=True, knative=True,
        input="{} {}".format(n_workers, interval),
        native=native,
    )

    # Pull the billing info
    pull_billing()

    if not success:
        print("FAILED on {} {}".format(n_workers, interval))

    output_dir = join(FAASM_HOME, "sgd_results")
    if not exists(output_dir):
        mkdir(output_dir)

    system = "NATIVE" if native else "FAASM"
    folder_name = "SYSTEM_{}_WORKERS_{}_INTERVAL_{}_logs".format(system, n_workers, interval)
    result_dir = join(output_dir, folder_name)
    if exists(result_dir):
        rmtree(result_dir)
    mkdir(result_dir)

    output_file = join(result_dir, "NODE_0_SGD_LOSS.log")
    with open(output_file, "w") as fh:
        fh.write(output)

    # Copy billing directory into place
    res = call("cp -r /tmp/billing {}/".format(result_dir))
    if res != 0:
        raise RuntimeError("Failed to put billing files in place")

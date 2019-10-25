from os import mkdir, listdir
from os.path import exists, join
from shutil import rmtree
from subprocess import call

from invoke import task

from tasks.util.billing import start_billing, pull_billing, parse_billing
from tasks.util.env import FAASM_HOME
from tasks.util.invoke import invoke_impl

LOCAL_RESULT_DIR = join(FAASM_HOME, "sgd_results")


@task
def sgd_experiment(ctx, workers, interval, native=False, both=False):
    # WASM run
    if not native:
        _do_call(workers, interval, False)

    # Native run
    if native or both:
        _do_call(workers, interval, True)


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
    res = call("cp -r /tmp/billing {}/".format(result_dir), shell=True)
    if res != 0:
        raise RuntimeError("Failed to put billing files in place")


@task
def sgd_pull_results(ctx, host, user):
    if exists(LOCAL_RESULT_DIR):
        rmtree(LOCAL_RESULT_DIR)

    cmd = "scp -r {}@{}:/home/{}/faasm/sgd_results {}".format(user, host, user, FAASM_HOME)

    call(cmd, shell=True)


@task
def sgd_parse_results(ctx):
    for dir_name in listdir(LOCAL_RESULT_DIR):
        print("Parsing results for {}".format(dir_name))
        parent_dir = join(LOCAL_RESULT_DIR, dir_name)
        billing_result_dir = join(parent_dir, "billing", "results")
        parse_billing(billing_result_dir, parent_dir)

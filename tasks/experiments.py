from abc import abstractmethod
from os import mkdir, listdir
from os.path import exists, join
from shutil import rmtree
from subprocess import call
from time import time, sleep

from invoke import task

from tasks import delete_knative_native_python, matrix_state_upload, delete_knative_worker, flush
from tasks.util.billing import start_billing, pull_billing, parse_billing
from tasks.util.env import FAASM_HOME
from tasks.util.invoke import invoke_impl


class ExperimentRunner(object):
    user = None
    func = None
    is_python = False

    poll_interval = 1000

    result_file_name = None
    local_results_dir = None

    def __init__(self, input_data=None):
        self.input_data = input_data
        self.no_billing = False

    @abstractmethod
    def get_result_dir_name(self, system):
        pass

    @classmethod
    def get_local_results_dir(cls):
        return join(FAASM_HOME, "{}_{}".format(cls.user, cls.func))

    @classmethod
    def clean(cls):
        result_dir = cls.get_local_results_dir()
        if exists(result_dir):
            rmtree(result_dir)

    @classmethod
    def pull_results(cls, host, host_user):
        cmd = "scp -r {}@{}:/home/{}/faasm/{}_{} {}".format(host_user, host, host_user, cls.user, cls.func, FAASM_HOME)
        print(cmd)
        call(cmd, shell=True)

    @classmethod
    def parse_results(cls):
        results_dir = cls.get_local_results_dir()
        for dir_name in listdir(results_dir):
            print("Parsing results for {}".format(dir_name))
            parent_dir = join(results_dir, dir_name)
            billing_result_dir = join(parent_dir, "billing", "results")
            parse_billing(billing_result_dir, parent_dir)

    def run(self, native, nobill=False):
        self.no_billing = nobill

        if native:
            self.run_native()
        else:
            self.run_wasm()

    def run_wasm(self):
        self._do_run(False)

    def run_native(self):
        self._do_run(True)

    def _do_run(self, native):
        if not self.no_billing:
            # Start the billing scripts
            start_billing()

        # Start the timer
        run_start = time()

        # Run the request
        success, output = invoke_impl(
            self.user, self.func,
            poll=True, poll_interval_ms=self.poll_interval,
            knative=True,
            input=self.input_data,
            native=native, py=self.is_python,
        )

        # Finish the timer
        run_time_ms = (time() - run_start) * 1000.0

        if not self.no_billing:
            # Pull the billing info
            pull_billing()

        if not success:
            print("FAILED on {}".format(self.input_data))

        output_dir = join(FAASM_HOME, "{}_{}".format(self.user, self.func))
        if not exists(output_dir):
            mkdir(output_dir)

        system = "NATIVE" if native else "FAASM"
        folder_name = self.get_result_dir_name(system)
        result_dir = join(output_dir, folder_name)
        if exists(result_dir):
            rmtree(result_dir)
        mkdir(result_dir)

        # Write the running time
        output_file = join(result_dir, "RUN_TIME.log")
        with open(output_file, "w") as fh:
            fh.write("{}ms".format(run_time_ms))

        # Write the output from the function
        output_file = join(result_dir, self.result_file_name)
        with open(output_file, "w") as fh:
            fh.write(output)

        # Copy billing directory into place
        if not self.no_billing:
            res = call("cp -r /tmp/billing {}/".format(result_dir), shell=True)
            if res != 0:
                raise RuntimeError("Failed to put billing files in place")


# -------------------------------------
# SGD
# -------------------------------------

class SGDExperimentRunner(ExperimentRunner):
    user = "sgd"
    func = "reuters_svm"
    result_file_name = "NODE_0_SGD_LOSS.log"

    def __init__(self, n_workers, interval):
        super().__init__("{} {}".format(n_workers, interval))

        self.n_workers = n_workers
        self.interval = interval

    def get_result_dir_name(self, system):
        folder_name = "SYSTEM_{}_WORKERS_{}_INTERVAL_{}_logs".format(system, self.n_workers, self.interval)
        return folder_name


@task
def sgd_experiment(ctx, workers, interval, native=False, nobill=False):
    runner = SGDExperimentRunner(workers, interval)
    runner.run(native, nobill=nobill)


@task
def sgd_pull_results(ctx, host, user):
    SGDExperimentRunner.clean()
    SGDExperimentRunner.pull_results(host, user)


@task
def sgd_parse_results(ctx):
    SGDExperimentRunner.parse_results()


# -------------------------------------
# Matrix multiplication
# -------------------------------------

class MatrixExperimentRunner(ExperimentRunner):
    user = "python"
    func = "mat_mul"
    is_python = True
    result_file_name = "NODE_0_MAT_MUL.log"

    poll_interval = 500

    def __init__(self, n_workers, mat_size, n_splits):
        super().__init__(None)

        self.n_workers = n_workers
        self.mat_size = mat_size
        self.n_splits = n_splits

    def get_result_dir_name(self, system):
        folder_name = "SYSTEM_{}_MATRIX_{}_SPLITS_{}_WORKERS_{}_logs".format(system, self.mat_size, self.n_splits,
                                                                             self.n_workers)
        return folder_name


@task
def matrix_experiment_multi(ctx, n_workers, native=False, nobill=False):
    sizes = [128, 512, 1024, 2048, 4096]
    splits = [0, 1, 2, 3]

    for mat_size in sizes:
        for n_splits in splits:
            if native:
                delete_knative_native_python(ctx, hard=False)
                sleep(40)
            else:
                print("\nFLUSHING\n")
                flush(ctx)
                sleep(30)

            print("\nUPLOADING STATE - {}x{} {}\n".format(mat_size, mat_size, n_splits))
            matrix_state_upload(ctx, mat_size, n_splits)

            print("\nRUNNING EXPERIMENT - {}x{} {}\n".format(mat_size, mat_size, n_splits))
            matrix_experiment(ctx, n_workers, mat_size, n_splits, native=native, nobill=nobill)


@task
def matrix_experiment(ctx, n_workers, mat_size, n_splits, native=False, nobill=False):
    runner = MatrixExperimentRunner(n_workers, mat_size, n_splits)
    runner.run(native, nobill=nobill)


@task
def matrix_pull_results(ctx, user, host):
    MatrixExperimentRunner.clean()
    MatrixExperimentRunner.pull_results(host, user)


@task
def matrix_parse_results(ctx):
    MatrixExperimentRunner.parse_results()

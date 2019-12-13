import os
import subprocess
from abc import abstractmethod
from decimal import Decimal
from os import mkdir, listdir, remove, makedirs
from os.path import exists, join
from shutil import rmtree
from subprocess import call
from time import time, sleep

from invoke import task

from tasks import delete_knative_native_python, delete_knative_worker, matrix_state_upload
from tasks.util.billing import start_billing, pull_billing, parse_billing
from tasks.util.endpoints import get_worker_host_port
from tasks.util.env import FAASM_HOME, PROJ_ROOT
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
    sizes = [100, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000]
    splits = [2]

    for mat_size in sizes:
        for n_splits in splits:
            if native:
                delete_knative_native_python(ctx, hard=False)
                sleep_time = 60
            else:
                delete_knative_worker(ctx, hard=False)
                sleep_time = 130

            print("\nUPLOADING STATE - {}x{} {}\n".format(mat_size, mat_size, n_splits))
            matrix_state_upload(ctx, mat_size, n_splits)

            sleep(sleep_time)

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

    MatrixExperimentRunner.parse_results()


# -------------------------------------
# Tensorflow
# -------------------------------------


class TensorflowExperimentRunner():
    def __init__(self, threads=4, connections_per_thread=4, delay_ms=0, duration_secs=5):
        self.wrk_bin = join(FAASM_HOME, "tools", "wrk")
        self.threads = threads
        self.connections_per_thread = connections_per_thread
        self.delay_ms = delay_ms

        self.host, self.port = get_worker_host_port(None, None)
        self.url = "http://{}:{}/f/tf/image".format(self.host, self.port)

        self.duration_secs = duration_secs
        self.wrk_output = "/tmp/wrk_results.csv"

        self.base_result_dir = join(FAASM_HOME, "tf_image")
        if not exists(self.base_result_dir):
            makedirs(self.base_result_dir)

    def run_native(self):
        self._run_wrk("NATIVE")

    def run_wasm(self):
        self._run_wrk("WASM")

    def _run_wrk(self, bench_mode):
        result_dir = join(self.base_result_dir, "SYSTEM_{}_THREADS_{}_CONNS_{}_DELAY_{}_logs".format(
            bench_mode, self.threads, self.connections_per_thread, self.delay_ms
        ))

        # Clean up and create results dir
        if exists(result_dir):
            rmtree(result_dir)
        makedirs(result_dir)

        if exists(self.wrk_output):
            remove(self.wrk_output)

        # Set up Lua script
        os.environ["BENCH_MODE"] = bench_mode
        os.environ["BENCH_DELAY_MS"] = str(self.delay_ms)
        script = join(PROJ_ROOT, "conf", "tflite_bench.lua")

        # Run wrk
        connections = self.threads * self.connections_per_thread
        cmd = [
            self.wrk_bin,
            "-t{}".format(self.threads),
            "-c{}".format(connections),
            "-s {}".format(script),
            "-d{}s".format(self.duration_secs),
            self.url,
        ]

        cmd = " ".join(cmd)
        print(cmd)
        res = subprocess.call(cmd, shell=True)
        if res != 0:
            raise RuntimeError("Failed running wrk")

        self._parse_results(result_dir)

    def _parse_results(self, result_dir):
        with open(self.wrk_output, "r") as fh:
            output_csv = fh.read()

        # Check we've got what we expect
        lines = output_csv.split("\n")
        lines = [l.strip() for l in lines if l.strip()]
        if len(lines) != 2:
            raise RuntimeError("Unexpected wrk output (lines = {})".format(len(lines)))

        results = lines[1]
        results = [Decimal(r.strip()) for r in results.split(",") if r.strip()]

        # Work out throughput
        duration = results[7] / Decimal(1000000)  # Seconds
        requests = results[8]
        tpt = requests / duration

        # Write to summary file
        # Note that all times are in microseconds so we want to convert them to millis
        result_file = join(result_dir, "latency.txt")
        with open(result_file, "w") as fh:
            fh.write("DURATION {}\n".format(duration))
            fh.write("REQUESTS {}\n".format(requests))
            fh.write("THROUGHPUT {}\n".format(tpt))
            fh.write("MIN {}\n".format(results[0] / Decimal(1000)))
            fh.write("MAX {}\n".format(results[1] / Decimal(1000)))
            fh.write("MEAN {}\n".format(results[2] / Decimal(1000)))
            fh.write("STDDEV {}\n".format(results[3] / Decimal(1000)))
            fh.write("MEDIAN {}\n".format(results[4] / Decimal(1000)))
            fh.write("90TH {}\n".format(results[5] / Decimal(1000)))
            fh.write("99TH {}\n".format(results[6] / Decimal(1000)))


@task
def tf_tpt_experiment_multi(ctx, native=False):
    delays = [0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000]
    for d in delays:
        runner = TensorflowExperimentRunner(
            threads=2,
            connections_per_thread=3,
            delay_ms=d,
            duration_secs=15,
        )

        if native:
            runner.run_native()
        else:
            runner.run_wasm()


@task
def tf_tpt_experiment(ctx, native=False):
    runner = TensorflowExperimentRunner(
        threads=2,
        connections_per_thread=4,
        delay_ms=0,
        duration_secs=10
    )

    if native:
        runner.run_native()
    else:
        runner.run_wasm()

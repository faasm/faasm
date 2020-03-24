import os
import subprocess
from abc import abstractmethod
from os import mkdir, listdir, remove
from os.path import exists, join
from shutil import rmtree, copy
from subprocess import call
from time import time, sleep

from invoke import task
from psutil import cpu_count

import tasks.data
import tasks.knative
from tasks.util.billing import start_billing, pull_billing, parse_billing
from tasks.util.billing_data import plot_billing_data_multi
from tasks.util.endpoints import is_kubernetes, get_invoke_host_port
from tasks.util.env import FAASM_HOME, PROJ_ROOT
from tasks.util.invoke import invoke_impl


class ExperimentRunner(object):
    user = None
    func = None
    is_python = False

    result_file_name = None
    local_results_dir = None

    def __init__(self, input_data=None):
        self.input_data = input_data
        self.no_billing = False

    @abstractmethod
    def get_result_folder_name(self, system):
        pass

    @abstractmethod
    def execute_benchmark(self):
        pass

    def get_result_dir(self, native):
        output_dir = join(FAASM_HOME, "{}_{}".format(self.user, self.func))
        if not exists(output_dir):
            mkdir(output_dir)

        system = "NATIVE" if native else "FAASM"
        folder_name = self.get_result_folder_name(system)
        result_dir = join(output_dir, folder_name)
        if exists(result_dir):
            rmtree(result_dir)
        mkdir(result_dir)

        return result_dir

    @classmethod
    def get_local_results_dir(cls):
        return join(FAASM_HOME, "{}_{}".format(cls.user, cls.func))

    @classmethod
    def clean(cls):
        result_dir = cls.get_local_results_dir()
        if exists(result_dir):
            rmtree(result_dir)

    @classmethod
    def pull_results(cls, host_user, host):
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

        # Set up result dir up front
        result_dir = self.get_result_dir(native)

        # Start the timer
        run_start = time()

        # Run the actual benchmark
        success, output = self.execute_benchmark(native)

        # Finish the timer
        run_time_ms = (time() - run_start) * 1000.0

        if not self.no_billing:
            # Pull the billing info
            pull_billing()

        if not success:
            print("FAILED on {}".format(self.input_data))

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


class InvokeAndWaitRunner(ExperimentRunner):
    poll_interval = 1000

    def execute_benchmark(self, native):
        success, output = invoke_impl(
            self.user, self.func,
            poll=True, poll_interval_ms=self.poll_interval,
            knative=True,
            input=self.input_data,
            native=native, py=self.is_python,
        )

        return success, output


class WrkRunner(ExperimentRunner):
    def __init__(self, threads=4, total_connections=20, delay_ms=0, duration_secs=10):
        super().__init__(None)

        self.wrk_bin = join(FAASM_HOME, "tools", "wrk")
        self.threads = threads
        self.total_connections = total_connections
        self.delay_ms = delay_ms

        self.host, self.port = get_invoke_host_port()
        self.url = "http://{}:{}".format(self.host, self.port)

        self.duration_secs = duration_secs
        self.wrk_output = "/tmp/wrk_results.txt"

    @abstractmethod
    def get_wrk_script(self):
        pass

    def get_result_folder_name(self, system):
        folder_name = "SYSTEM_{}_THREADS_{}_CONNS_{}_DELAY_{}_logs".format(
            system, self.threads, self.total_connections, self.delay_ms
        )

        return folder_name

    def execute_benchmark(self, native):
        result_dir = self.get_result_dir(native)

        if exists(self.wrk_output):
            remove(self.wrk_output)

        # Set up Lua script
        system = "NATIVE" if native else "FAASM"
        os.environ["BENCH_MODE"] = system
        os.environ["BENCH_DELAY_MS"] = str(self.delay_ms)
        script = self.get_wrk_script()

        # Run wrk
        cmd = [
            self.wrk_bin,
            "-t{}".format(self.threads),
            "-c{}".format(self.total_connections),
            "-s {}".format(script),
            "-d{}s".format(self.duration_secs),
            "--timeout=5s",
            self.url,
        ]

        cmd = " ".join(cmd)
        print(cmd)
        res = subprocess.call(cmd, shell=True)
        if res != 0:
            raise RuntimeError("Failed running wrk")

        self._parse_results(result_dir)

        return True, "No output"

    def _parse_results(self, result_dir):
        # Copy wrk output into place
        result_file = join(result_dir, "latency.txt")
        copy(self.wrk_output, result_file)


# -------------------------------------
# SGD
# -------------------------------------

class SGDExperimentRunner(InvokeAndWaitRunner):
    user = "sgd"
    func = "reuters_svm"
    result_file_name = "NODE_0_SGD_LOSS.log"

    def __init__(self, n_workers, interval):
        super().__init__("{} {}".format(n_workers, interval))

        self.n_workers = n_workers
        self.interval = interval

    def get_result_folder_name(self, system):
        folder_name = "SYSTEM_{}_WORKERS_{}_INTERVAL_{}_logs".format(system, self.n_workers, self.interval)
        return folder_name


@task
def sgd(ctx, workers, interval, native=False, nobill=False):
    """
    Run SGD experiment
    """
    runner = SGDExperimentRunner(workers, interval)
    runner.run(native, nobill=nobill)


@task
def sgd_pull_results(ctx, user, host):
    """
    Pull SGD experiment results
    """
    SGDExperimentRunner.clean()
    SGDExperimentRunner.pull_results(user, host)


@task
def sgd_parse_results(ctx):
    """
    Parse SGD experiment results
    """
    SGDExperimentRunner.parse_results()


# -------------------------------------
# Matrix multiplication
# -------------------------------------

class MatrixExperimentRunner(InvokeAndWaitRunner):
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

    def get_result_folder_name(self, system):
        folder_name = "SYSTEM_{}_MATRIX_{}_SPLITS_{}_WORKERS_{}_logs".format(system, self.mat_size, self.n_splits,
                                                                             self.n_workers)
        return folder_name


@task
def matrix_multi(ctx, n_workers, native=False, nobill=False):
    """
    Run multiple matrix experiment configurations
    """
    sizes = [100, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000]
    splits = [2]

    for mat_size in sizes:
        for n_splits in splits:
            if native:
                tasks.knative.delete_native_python(ctx, hard=False)
                sleep_time = 60
            else:
                tasks.knative.delete_worker(ctx, hard=False)
                sleep_time = 130

            print("\nUPLOADING STATE - {}x{} {}\n".format(mat_size, mat_size, n_splits))
            tasks.data.matrix_state(ctx, mat_size, n_splits)

            sleep(sleep_time)

            print("\nRUNNING EXPERIMENT - {}x{} {}\n".format(mat_size, mat_size, n_splits))
            matrix(ctx, n_workers, mat_size, n_splits, native=native, nobill=nobill)


@task
def matrix(ctx, n_workers, mat_size, n_splits, native=False, nobill=False):
    """
    Run matrix experiment
    """
    runner = MatrixExperimentRunner(n_workers, mat_size, n_splits)
    runner.run(native, nobill=nobill)


@task
def matrix_pull_results(ctx, user, host):
    """
    Pull matrix experiment results
    """
    MatrixExperimentRunner.clean()
    MatrixExperimentRunner.pull_results(user, host)

    MatrixExperimentRunner.parse_results()


# -------------------------------------
# Tensorflow
# -------------------------------------


class TensorflowExperimentRunner(WrkRunner):
    user = "tf"
    func = "image"
    is_python = False
    result_file_name = "NODE_0_INFERENCE.log"

    def __init__(self, cold_start_interval, threads=4, total_connections=20, delay_ms=5, duration_secs=10):
        super().__init__(
            threads=threads, total_connections=total_connections, delay_ms=delay_ms, duration_secs=duration_secs
        )

        self.cold_start_interval = cold_start_interval

    def execute_benchmark(self, native):
        os.environ["COLD_START_INTERVAL"] = str(self.cold_start_interval)

        return super().execute_benchmark(native)

    def get_wrk_script(self):
        return join(PROJ_ROOT, "deploy", "conf", "tflite_bench.lua")

    def get_result_folder_name(self, system):
        folder_name = "SYSTEM_{}_COLD_{}_THREADS_{}_CONNS_{}_DELAY_{}_logs".format(
            system, self.cold_start_interval, self.threads, self.total_connections, self.delay_ms
        )

        return folder_name


@task
def tf_lat(ctx):
    """
    Run TF latency experiment
    """

    # Aim of this experiment is to show how latency changes at low load with varying
    # numbers of cold starts. We don't want any interference so just run one thread
    # and one connection
    threads = 1
    total_connections = 1

    for native in [True, False]:
        if native:
            runs = [
                (5, 480),
                (50, 300),
                (500, 180),
            ]
        else:
            runs = [
                (500, 200),
            ]

        for cold_start_interval, duration_s in runs:
            # Run a ramp-up
            runner = TensorflowExperimentRunner(
                cold_start_interval,
                threads=threads,
                total_connections=total_connections,
                delay_ms=5,
                duration_secs=10,
            )

            runner.execute_benchmark(native)

            # Run the full thing
            runner = TensorflowExperimentRunner(
                cold_start_interval,
                threads=threads,
                total_connections=total_connections,
                delay_ms=5,
                duration_secs=duration_s,
            )

            runner.run(native, nobill=True)

            if not is_kubernetes():
                sleep(5)
                continue

            # Tidy up
            if native:
                tasks.knative.delete_native(ctx, "tf", "image", hard=False)
            else:
                tasks.knative.delete_worker(ctx, hard=False)

            sleep(30)


@task
def tf_tpt(ctx, native=False, nobill=False):
    """
    Run TF throughput experiment
    """

    # Runs with delay, duration
    runs = [
        (30000, 180),
        (20000, 160),
        (10000, 140),
        (5000, 120),
        (3000, 100),
        (2000, 100),
        # (1000, 100),
        # (800, 80),
        # (600, 80),
        # (400, 60),
        # (200, 60),
        # (100, 60),
        # (0, 60),
    ]

    # Different cold start intervals
    cold_start_intervals = [5, 10, 20, 40, 80] if native else [500]
    threads = cpu_count()
    total_connections = 100

    for cold_start_interval in cold_start_intervals:
        for delay_ms, duration_s in runs:
            # Run a ramp-up
            # runner = TensorflowExperimentRunner(
            #     cold_start_interval,
            #     threads=threads,
            #     total_connections=20,
            #     delay_ms=delay_ms,
            #     duration_secs=15,
            # )
            # runner.execute_benchmark(native)

            # Run the full thing
            runner = TensorflowExperimentRunner(
                cold_start_interval,
                threads=threads,
                total_connections=total_connections,
                delay_ms=delay_ms,
                duration_secs=duration_s,
            )

            runner.run(native, nobill=nobill)

            if not is_kubernetes():
                sleep(5)
                continue

            # Tidy up
            if native:
                tasks.knative.delete_native(ctx, "tf", "image", hard=False)
            else:
                tasks.knative.delete_worker(ctx, hard=False)

            sleep_time = 40
            sleep(sleep_time)


@task
def tf_plot_billing(ctx, result_dir):
    """
    Plot billing data from TF experiment
    """
    plot_billing_data_multi(result_dir)


@task
def tf_lat_pull_results(ctx, user, host):
    """
    Pull results for TF latency experiment
    """
    TensorflowExperimentRunner.pull_results(user, host)
    # No parsing for latency results


@task
def tf_tpt_pull_results(ctx, user, host, nobill=False):
    """
    Pull results for TF throughput experiment
    """
    TensorflowExperimentRunner.pull_results(user, host)

    if not nobill:
        TensorflowExperimentRunner.parse_results()


@task
def tf_tpt_parse_results(ctx):
    """
    Parse results for TF throughput experiment
    """
    TensorflowExperimentRunner.parse_results()

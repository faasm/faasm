import os
from copy import copy
from os import mkdir, remove, makedirs
from os.path import join, exists
from subprocess import check_output, call
from time import sleep

from invoke import task

from tasks.util.endpoints import get_upload_host_port, get_invoke_host_port
from tasks.util.env import FAASM_DATA_DIR, THIRD_PARTY_DIR, FAASM_SHARED_STORAGE_ROOT
from tasks.util.genomics import GENOMICS_DATA_DIR, CHROMOSOME_URLS, CHROMOSOME_NUMBERS, READ_URLS, get_reads_from_dir, \
    INDEX_CHUNKS
from tasks.util.invoke import invoke_impl, status_call_impl, STATUS_SUCCESS, STATUS_FAILED, STATUS_RUNNING
from tasks.util.state import download_binary_state
from tasks.util.state import upload_shared_file


@task
def mapping(ctx):
    read_idxs, _ = get_reads_from_dir()

    # Iterate through and make the calls to the worker
    call_ids = list()
    for read_idx in read_idxs:
        print("Invoking worker for read chunk {}".format(read_idx))
        call_id = invoke_impl("gene", "mapper", input="{}".format(read_idx), async=True, poll=False)
        call_ids.append(call_id)

    # Poll for completion of each read
    completed_read_idxs = list()
    host, port = get_invoke_host_port()
    print("Polling workers...")

    while len(completed_read_idxs) < len(read_idxs):
        for i, read_idx in enumerate(read_idxs):
            sleep(0.5)

            # See whether this call is still running
            call_id = call_ids[i]
            result, output = status_call_impl(call_id, host, port)
            if result == STATUS_RUNNING:
                continue

            # Check for success or failure
            if result == STATUS_SUCCESS:
                # Download the results of this read
                print("Read chunk {} completed. Downloading output".format(read_idx))
                state_key = "output_read_{}".format(read_idx)

                output_dir = join(FAASM_DATA_DIR, "genomics_output")
                if not exists(output_dir):
                    makedirs(output_dir)

                output_file = join(output_dir, state_key)
                host, port = get_upload_host_port(None, None)
                download_binary_state("gene", state_key, output_file, host=host, port=port)

            elif result == STATUS_FAILED:
                print("Read chunk {} failed: {}", read_idx, output)

            # Check if we're done
            completed_read_idxs.append(read_idx)

    print("All read chunks finished")


@task
def download_genome(ctx):
    if not exists(GENOMICS_DATA_DIR):
        mkdir(GENOMICS_DATA_DIR)

    for url in CHROMOSOME_URLS:
        zip_filename = url.split("/")[-1]

        # Download the file
        download_file = join(GENOMICS_DATA_DIR, zip_filename)
        print("URL: {} -> {}\n".format(url, download_file))
        check_output("wget {} -O {}".format(url, download_file), shell=True)

        # Extract zip
        print("Extracting {}".format(download_file))
        check_output("gunzip -f {}".format(download_file), shell=True, cwd=GENOMICS_DATA_DIR)

        # Check unzipping
        filename = zip_filename.replace(".gz", "")
        unzipped_file = join(GENOMICS_DATA_DIR, filename)
        if not exists(unzipped_file):
            print("Didn't find unzipped file at {} as expected.".format(unzipped_file))
            exit(1)

        print("Unzipped file at {}".format(unzipped_file))


@task
def index_genome(ctx):
    work_dir = join(THIRD_PARTY_DIR, "gem3-mapper")
    shell_env = copy(os.environ)
    shell_env["LD_LIBRARY_PATH"] = "/usr/local/lib:{}".format(shell_env.get("LD_LIBRARY_PATH", ""))

    binary = join(work_dir, "bin", "gem-indexer")
    if not exists(binary):
        raise RuntimeError("Expected to find executable at {}".format(binary))

    for idx, name in enumerate(CHROMOSOME_NUMBERS):
        input_file = join(FAASM_DATA_DIR, "genomics", "Homo_sapiens.GRCh38.dna.chromosome.{}.fa".format(name))
        output_file = join(FAASM_DATA_DIR, "genomics", "index_{}".format(idx))

        cmd = [
            binary,
            "-i {}".format(input_file),
            "-o {}".format(output_file),
        ]

        cmd_str = " ".join(cmd)
        print(cmd_str)
        call(cmd_str, shell=True, env=shell_env, cwd=work_dir)

        # Remove unnecessary files
        info_file = "{}.info".format(output_file)
        remove(input_file)
        remove(info_file)


@task
def download_reads(ctx):
    if not exists(GENOMICS_DATA_DIR):
        mkdir(GENOMICS_DATA_DIR)

    for url in READ_URLS:
        filename = url.split("/")[-1]
        download_file = join(GENOMICS_DATA_DIR, filename)
        print("URL: {} -> {}\n".format(url, download_file))
        check_output("wget {} -O {}".format(url, download_file), shell=True)


@task
def upload_data(ctx, host="localhost", local_copy=False):
    dest_root = join(FAASM_SHARED_STORAGE_ROOT, "genomics")
    if local_copy and not exists(dest_root):
        makedirs(dest_root)

    files_to_upload = list()

    read_idxs, file_paths = get_reads_from_dir()
    for read_idx, file_path in zip(read_idxs, file_paths):
        files_to_upload.append((file_path, "reads_{}.fq".format(read_idx)))

    for index_chunk in INDEX_CHUNKS:
        filename = "index_{}.gem".format(index_chunk)
        file_path = join(FAASM_DATA_DIR, "genomics", filename)
        files_to_upload.append((file_path, filename))

    for file_path, file_name in files_to_upload:
        if local_copy:
            dest_file = join(dest_root, file_name)
            call("cp {} {}".format(file_path, dest_file), shell=True)
        else:
            shared_path = "genomics/{}".format(file_name)
            upload_shared_file(host, file_path, shared_path)

from os import listdir
from os.path import join
from time import sleep

from invoke import task

from tasks.upload import _get_host_port
from tasks.util.env import FAASM_DATA_DIR
from tasks.util.invoke import invoke_impl, status_call_impl, STATUS_SUCCESS, STATUS_FAILED, STATUS_RUNNING
from tasks.util.state import upload_binary_state, download_binary_state


def _get_reads_from_dir():
    # We expect reads data to exist at the following location and be divided up into files
    # reads_1, reads_2, reads_3 etc.

    # Iterate through the files, work out what we have
    read_idxs = list()
    file_paths = list()
    reads_dir = join(FAASM_DATA_DIR, "genomics", "reads")
    for reads_filename in listdir(reads_dir):
        if not reads_filename.startswith("reads_"):
            print("Skipping file {}".format(reads_filename))
            continue

        # Work out which chunk we're dealing with
        filename_parts = reads_filename.split("_")
        filename_parts = [f.strip() for f in filename_parts if f.strip()]
        if len(filename_parts) != 2:
            print("Invalid reads filename: {}".format(reads_filename))
            continue

        read_idx = int(filename_parts[1])
        read_idxs.append(read_idx)

        file_path = join(reads_dir, reads_filename)
        file_paths.append(file_path)

        print("Found reads file at {} for read chunk {}".format(file_path, read_idx))

    return read_idxs, file_paths


@task
def upload_reads(ctx):
    read_idxs, file_paths = _get_reads_from_dir()

    for read_idx, file_path in zip(read_idxs, file_paths):
        print("Uploading reads file at {} for read chunk {}".format(file_path, read_idx))
        upload_binary_state("gene", "reads_{}".format(read_idx), file_path)


@task
def mapping(ctx):
    read_idxs, _ = _get_reads_from_dir()

    # Iterate through and make the calls to the worker
    call_ids = list()
    for read_idx in read_idxs:
        call_id = invoke_impl("gene", "mapper", input="{}".format(read_idx), async=True, poll=False)
        call_ids.append(call_id)

    # Poll for completion of each read
    completed_read_idxs = list()
    for i, read_idx in enumerate(read_idxs):
        sleep(0.5)

        # See whether this call is still running
        call_id = call_ids[i]
        result, output = status_call_impl(call_id)
        if result == STATUS_RUNNING:
            continue

        # Check for success or failure
        if result == STATUS_SUCCESS:
            # Retrieve output from state
            pass
        elif result == STATUS_FAILED:
            print("Read chunk {} failed: {}", read_idx, output)

        # Download the results of this read
        state_key = "output_read_{}".format(read_idx)
        output_file = join(FAASM_DATA_DIR, "genomics_output", state_key)
        host, port = _get_host_port(None, None)
        download_binary_state("gene", state_key, output_file, host=host, port=port)

        # Check if we're done
        completed_read_idxs.append(read_idx)
        if len(completed_read_idxs) == len(read_idxs):
            break

    print("All read chunks finished")

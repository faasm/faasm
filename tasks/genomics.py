from os import makedirs
from os.path import join, exists
from time import sleep

from invoke import task

from tasks.util.endpoints import get_kubernetes_host_port, get_worker_host_port
from tasks.util.endpoints import get_upload_host_port
from tasks.util.env import FAASM_DATA_DIR
from tasks.util.genomics import get_reads_from_dir
from tasks.util.invoke import invoke_impl, status_call_impl, STATUS_SUCCESS, STATUS_FAILED, STATUS_RUNNING
from tasks.util.state import download_binary_state


@task
def genomics_mapping(ctx):
    read_idxs, _ = get_reads_from_dir()

    # Iterate through and make the calls to the worker
    call_ids = list()
    for read_idx in read_idxs:
        print("Invoking worker for read chunk {}".format(read_idx))
        call_id = invoke_impl("gene", "mapper", input="{}".format(read_idx), async=True, poll=False)
        call_ids.append(call_id)

    # Poll for completion of each read
    completed_read_idxs = list()
    host, port = get_worker_host_port(None, None)
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

from os.path import join
from time import sleep

from invoke import task

from tasks.upload import get_upload_host_port
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
        host, port = get_upload_host_port(None, None)
        download_binary_state("gene", state_key, output_file, host=host, port=port)

        # Check if we're done
        completed_read_idxs.append(read_idx)
        if len(completed_read_idxs) == len(read_idxs):
            break

    print("All read chunks finished")

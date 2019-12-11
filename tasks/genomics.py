from os import listdir
from os.path import join

from invoke import task

from tasks.util.env import FAASM_DATA_DIR
from tasks.util.state import upload_binary_state


@task
def mapping(ctx):
    # We expect reads data to exist at the following location and be divided up into files
    # reads_1, reads_2, reads_3 etc.

    # Iterate through the files, work out what we have and upload
    read_idxs = list()
    reads_dir = join(FAASM_DATA_DIR, "genomics", "reads")
    for reads_filename in listdir(reads_dir):
        if not reads_filename.startswith("reads_"):
            print("Skipping file {}".format(reads_filename))
            continue

        # Work out which chunks we're dealing with
        filename_parts = reads_filename.split("_")
        filename_parts = [f.strip() for f in filename_parts if f.strip()]
        if len(filename_parts) != 2:
            print("Invalid reads filename: {}".format(reads_filename))
            continue

        # Upload the file to state
        read_idx = int(filename_parts[1])
        read_idxs.append(read_idx)
        print("Uploading reads file {} for read chunk {}".format(reads_filename, read_idx))
        file_path = join(reads_dir, reads_filename)
        upload_binary_state("gene", "reads_{}".format(read_idx), file_path)

    # TODO - Invoke the relevant function for each read idx

    # TODO - Poll for the completion of each one

    # TODO - When one finishes, download its output

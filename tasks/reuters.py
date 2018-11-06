from os.path import join

from invoke import task

from tasks.download import download_proj

URL = "https://archive.ics.uci.edu/ml/machine-learning-databases/00259/rcv1rcv2aminigoutte.tar.bz2"

# Vocabulary size
N_FEATURES = 21531
TARGET_CATEGORY = "CCAT"


@task
def reuters_download(ctx):
    extract_dir, build_dir = download_proj(
        URL,
        "rcv1rcv2aminigoutte",
        extension="tar.bz2"
    )

    raw_fh = open(join(extract_dir, "EN", "Index_EN-EN"), "r")
    inputs_fh = open(join(extract_dir, "inputs.txt"), "wb")
    categories_fh = open(join(extract_dir, "categories.txt"), "wb")

    # Each line has format of:
    # <cat> <idx>:<count> <idx>:<count> <idx>:<count> ...
    # E.g.
    # ECAT 1:-4.267123 3:1.233134 4:1.313571 5:1.644876 10:2.773822 21:3.362793 ...
    line_count = 0
    for line in raw_fh:
        line_parts = line.split(" ")

        # Remove gaps
        line_parts = [l.strip() for l in line_parts if l.strip()]

        # Work out if this is in our target category (1) or not (-1) and write the result
        # to the relevant file
        this_category = line_parts[0]
        this_class = 1 if this_category == TARGET_CATEGORY else -1
        categories_fh.write("{}".format(this_class).encode())
        categories_fh.write("\n".encode())

        # Build array of inputs with zeroes for gaps
        this_inputs = [0.0] * N_FEATURES

        # Iterate through parts of the line to pull out values
        for line_part in line_parts[1:]:
            tokens = line_part.split(":")
            assert len(tokens) == 2, "Unrecognised format {}".format(line_part)

            # Use 0-indexed (data is 1-indexed)
            idx = int(tokens[0]) - 1

            count = float(tokens[1])

            this_inputs[idx] = count

        # Write out the full sparse matrix with zeroes
        for i in this_inputs:
            inputs_fh.write("{0:.4f} ".format(i).encode())

        # Line break for next input line
        inputs_fh.write("\n".encode())

        line_count += 1
        if line_count > 0 and line_count % 100 == 0:
            print("Processed {} lines".format(line_count))

    categories_fh.flush()
    inputs_fh.flush()

    raw_fh.close()
    categories_fh.close()
    inputs_fh.close()

    print("Finished processing a total of {} lines".format(line_count))

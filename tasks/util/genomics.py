from os import listdir
from os.path import join
from tasks.util.env import FAASM_DATA_DIR

# Reads are sequences of indexed nucleotides and are the input to the mapper
# Not sure what the best source is so far, but have found some examples

# Genome data can be found at ftp://ftp-trace.ncbi.nih.gov/genomes
# The top-level README describes the layout (ftp://ftp.ncbi.nih.gov/genomes/README.txt)
# In general you are looking for a file with type `.fasta` or `.fa` or `.fna`

# The full human genome is here: ftp://ftp-trace.ncbi.nih.gov/1000genomes/ftp/technical/reference/human_g1k_v37.fasta.gz
# We download individual chromosomes from here: ftp://ftp.ensembl.org/pub/release-97/fasta/homo_sapiens/dna/
#
# A puffer fish genome can be found here:
# ftp://ftp-trace.ncbi.nih.gov/genomes/genbank/vertebrate_other/Tetraodon_nigroviridis/representative/GCA_000180735.1_ASM18073v1/GCA_000180735.1_ASM18073v1_genomic.fna.gz

READ_URLS = [
    "https://raw.githubusercontent.com/BenLangmead/bowtie2/master/example/reads/reads_1.fq",
]

# Python ranges are exclusive, so we need chromosomes 1-22 and X and Y for 24 in total
CHROMOSOME_NUMBERS = [str(i) for i in range(1, 23)]
CHROMOSOME_NUMBERS.append("X")
CHROMOSOME_NUMBERS.append("Y")

# TEMP - hack this
# INDEX_CHUNKS = range(1, 25)
INDEX_CHUNKS = range(1, 3)

CHROMOSOME_URLS = [
    "ftp://ftp.ensembl.org/pub/release-97/fasta/homo_sapiens/dna/Homo_sapiens.GRCh38.dna.chromosome.{}.fa.gz".format(i)
    for i in CHROMOSOME_NUMBERS
]

GENOMICS_DATA_DIR = join(FAASM_DATA_DIR, "genomics")


def get_reads_from_dir():
    # We expect reads data to exist at the following location and be divided up into files
    # reads_1.fq, reads_2.fq, reads_3.fq etc.

    # Iterate through the files, work out what we have
    read_idxs = list()
    file_paths = list()
    reads_dir = join(FAASM_DATA_DIR, "genomics")
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

        read_idx_str = filename_parts[1].replace(".fq", "")
        read_idx = int(read_idx_str)
        read_idxs.append(read_idx)

        file_path = join(reads_dir, reads_filename)
        file_paths.append(file_path)

        print("Found reads file at {} for read chunk {}".format(file_path, read_idx))

    return read_idxs, file_paths

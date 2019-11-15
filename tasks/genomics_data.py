import os
from copy import copy
from os import mkdir, remove
from os.path import join, exists
from subprocess import check_output, call

from invoke import task

from tasks.util.env import FAASM_DATA_DIR, THIRD_PARTY_DIR

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

CHROMOSOME_NUMBERS = range(1, 22)

CHROMOSOME_URLS = [
    "ftp://ftp.ensembl.org/pub/release-97/fasta/homo_sapiens/dna/Homo_sapiens.GRCh38.dna.chromosome.{}.fa.gz".format(i)
    for i in CHROMOSOME_NUMBERS
]

GENOMICS_DATA_DIR = join(FAASM_DATA_DIR, "genomics")


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

    for chromosome_idx in CHROMOSOME_NUMBERS:
        input_file = join(FAASM_DATA_DIR, "genomics", "Homo_sapiens.GRCh38.dna.chromosome.{}.fa".format(chromosome_idx))
        output_file = join(FAASM_DATA_DIR, "genomics", "index_{}".format(chromosome_idx))

        cmd = [
            binary,
            "-i {}".format(input_file),
            "-o {}".format(output_file),
        ]

        cmd_str = " ".join(cmd)
        print(cmd_str)
        call(cmd_str, shell=True, env=shell_env, cwd=work_dir)


@task
def download_reads(ctx):
    if not exists(GENOMICS_DATA_DIR):
        mkdir(GENOMICS_DATA_DIR)

    for url in READ_URLS:
        filename = url.split("/")[-1]
        download_file = join(GENOMICS_DATA_DIR, filename)
        print("URL: {} -> {}\n".format(url, download_file))
        check_output("wget {} -O {}".format(url, download_file), shell=True)

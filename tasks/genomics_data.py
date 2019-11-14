from os import mkdir
from os.path import join, exists
from subprocess import check_output

from invoke import task

from tasks.util.env import FAASM_HOME, FAASM_DATA_DIR

# Reads are sequences of indexed nucleotides and are the input to the mapper
# Not sure what the best source is so far, but have found some examples

# Genome data can be found at ftp://ftp-trace.ncbi.nih.gov/genomes
# The top-level README describes the layout (ftp://ftp.ncbi.nih.gov/genomes/README.txt)
# In general you are looking for a file with type `.fasta` or `.fa` or `.fna`

SOURCES = [
    "https://raw.githubusercontent.com/BenLangmead/bowtie2/master/example/reads/reads_1.fq",
]

GENOMES = {
    "human": {
        "url": "ftp://ftp-trace.ncbi.nih.gov/1000genomes/ftp/technical/reference/human_g1k_v37.fasta.gz"
    },
    "puffer": {
        "url": "ftp://ftp-trace.ncbi.nih.gov/genomes/genbank/vertebrate_other/Tetraodon_nigroviridis/representative/GCA_000180735.1_ASM18073v1/GCA_000180735.1_ASM18073v1_genomic.fna.gz",
    },
    "human-c-1": {
        "url": "ftp://ftp.ensembl.org/pub/release-97/fasta/homo_sapiens/dna/Homo_sapiens.GRCh38.dna.chromosome.1.fa.gz",
    },
    "human-c-20": {
        "url": "ftp://ftp.ensembl.org/pub/release-97/fasta/homo_sapiens/dna/Homo_sapiens.GRCh38.dna.chromosome.20.fa.gz"
    }
}

GENOMICS_DATA_DIR = join(FAASM_DATA_DIR, "genomics")


@task
def download_genome(ctx, species):
    if species not in GENOMES.keys():
        print("Species not regonised (options = {})".format(GENOMES.keys()))
        exit(1)

    if not exists(GENOMICS_DATA_DIR):
        mkdir(GENOMICS_DATA_DIR)

    genome = GENOMES[species]
    url = genome["url"]
    zip_filename = url.split("/")[-1]

    download_file = join(GENOMICS_DATA_DIR, zip_filename)
    print("Downloading {} genome:\nURL: {}\ndest: {}\n".format(species, url, download_file))
    check_output("wget {} -O {}".format(url, download_file), shell=True)

    print("Extracting {}".format(download_file))
    check_output("gunzip -f {}".format(download_file), shell=True, cwd=GENOMICS_DATA_DIR)

    filename = zip_filename.replace(".gz", "")
    unzipped_file = join(GENOMICS_DATA_DIR, filename)
    if not exists(unzipped_file):
        print("Didn't find unzipped file at {} as expected.".format(unzipped_file))
        exit(1)

    print("Unzipped file at {}".format(unzipped_file))


@task
def download_reads(ctx):
    if not exists(GENOMICS_DATA_DIR):
        mkdir(GENOMICS_DATA_DIR)

    for url in SOURCES:
        filename = url.split("/")[-1]
        download_file = join(GENOMICS_DATA_DIR, filename)
        print("Downloading reads:\nURL: {}\ndest: {}\n".format(url, download_file))
        check_output("wget {} -O {}".format(url, download_file), shell=True)

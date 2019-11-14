# Usage Notes

Need to make sure Faasm libraries are built and set up on system (see Faasm repo for details).

Start off anything by running:

```
source workon.sh
```

## Data

You can download a genome with the `download_genome.py` script, e.g.

```
python3 download_genome.py human-c-20
```

You can then index it with:

```
./bin/gem-indexer -i data/Homo_sapiens.GRCh38.dna.chromosome.20.fa -o data/human_c_20_idx.gem
```

You can download reads to index using the `download_reads.py` script:

```
python3 download_reads.py
```

## Mapping

To map a reads file you can do the following:

```
./bin/gem-mapper -I data/human_c_20_idx.gem -i data/reads_1.fq -o data/my_output.sam
```

You can change threads with `-t`. Adding `-t 1` can be useful for debugging.

## WASM

Once you've built and uploaded the function you can invoke it as normal (user is `gene` and function is called `mapper`).

## Misc

Lots of animal genomes at [this FTP server](ftp://ftp-trace.ncbi.nih.gov/genomes/).

See the readme for the file layout. Can add more in the `download_genome.py` script.

This page also has stuff: https://www.ensembl.org/Homo_sapiens/Info/Index (good for individual chromosomes).

# Building

**Note: if building native and wasm in the same directory, be sure to clean when switching**

## Native

To run locally:

```
# Native build
./native_build.sh
```

## WASM

To build to WASM and upload to Faasm:

```
# Set up wasm symlink for filesystem (debug)
./wasm_filesystem.sh

# Make clean essential if you've been running native builds
make clean
./wasm_clean.sh

# Run the build and upload
./wasm_build.sh
./wasm_upload.sh
```

## CLion

- Add a new native toolchain (Settings -> Build, Execution, Deployment -> Toolchains)
- Add a new custom build target (along with a new build tool for `make` under the "build" field)
- Create a new run configuration for this target
- Have it run `bin/gem-indexer` with the relevant input/ output files

# Internals

Mapping is handled through `mapper.c` which calls `mapper_run`. For each thread it creates a
`mapping_stats_t` and a `mapper_search_t`.

Threads are either a `mapper_pe_thread` or a `mapper_se_thread`, these are just different
types of mapping and also live in `mapper.c`. `mapper_se_thread` is default.

The mapper parameters tell each thread which files it's dealing with, which thread number
it is etc.

Concerns/ challenges:

- Threads share several pointers around parameters and I/O
- Worth trying to find out which ones are needed and which aren't

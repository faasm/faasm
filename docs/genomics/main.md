# Genomics Use-case

If building native and wasm in the same directory, be sure to clean when switching.

## Native

First you need to install libfaasm natively:

```
inv install-native-tools
```

One that's set up, you can run the following:

```
cd third-party/gem3-mapper
./native_build.sh
```

The repo itself then describes how to use this code.

### Data and Indexing

The index and reads only need to be set up once and uploaded to S3. To do this you need a native build of the indexer (described above). Then you can run:

```
# Download the data
inv download-genome human-c-20
inv download-reads

# Run the indexing
export LD_LIBRARY_PATH=/usr/local/lib
./bin/gem-indexer -i ~/faasm/data/genomics/Homo_sapiens.GRCh38.dna.chromosome.20.fa -o ~/faasm/data/genomics/human_c_20_idx.gem

# Do the upload
inv genomics_upload_s3
```

### Mapping

To map a reads file you can do the following:

```
./bin/gem-mapper -I data/human_c_20_idx.gem -i data/reads_1.fq -o data/my_output.sam
```

You can change threads with `-t`. Adding `-t 1` can be useful for debugging.

## WASM

To build for WASM, you can run:

```
cd third-party/gem3-mapper

# Set up wasm symlink for filesystem (debug)
./wasm_filesystem.sh

# Build
make clean
./wasm_clean.sh
./wasm_build.sh
```

Once you've built and uploaded the function you can invoke it as normal (user is `gene` and function is called `mapper`).

## Misc

Lots of animal genomes at [this FTP server](ftp://ftp-trace.ncbi.nih.gov/genomes/).

See the readme for the file layout. Can add more in the `download_genome.py` script.

This page also has stuff: https://www.ensembl.org/Homo_sapiens/Info/Index (good for individual chromosomes).

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

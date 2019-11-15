# Genomics Use-case

## Data

You can download the genomics data then upload with:

```
inv genomics-download-s3

# Use --local-copy if running locally
inv genomics-upload-data --local-copy
```

## WASM

To build and upload WASM, you can run:

```
# Build
./bin/clean_genomics.sh
./bin/build_genomics.sh

# Upload
inv upload-genomics

# Invoke
inv invoke gene mapper
```

## Native

Note, if you're building native and wasm in the same directory, be sure to clean when switching.

First you need to install libfaasm natively:

```
inv install-native-tools
```

One that's set up, you can run the following:

```
./bin/build_genomics_native.sh
```

The repo itself then describes how to use this code.

### Data and Indexing

The index and reads only need to be set up once and uploaded to S3. To do this you need a native build of the indexer (described above). Then you can run:

```
# Download the data
inv download-reads
inv download-genome

# Run the indexing
inv index-genome

# Do the upload
inv genomics-upload-s3
```

### Mapping

To map a reads file you can do the following:

```
./bin/gem-mapper -I data/human_c_20_idx.gem -i data/reads_1.fq -o data/my_output.sam
```

You can change threads with `-t`. Adding `-t 1` can be useful for debugging.

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

# Genetics

Some simple genetics notes for those who don't know what's going on.

- **DNA** - carries genetic information in chains of molecules called nucleotides. The nucleotides in DNA contain one of four bases: adenine, thymine, cytosine and guanine. These are represented by their first letters, i.e. A, T, C, G).
- **Gene** - a portion of DNA, roughly an instruction for making what an organism needs
- **Chromosome** - a bundle of genes. Humans have 46 chromosomes in 23 pairs
- **Locus** - a fixed position on a chromosome, e.g. marking the position of a gene
- **Protein** - chain of amino acids that forms the building blocks for life
- **Genome** - listing of the genetic material of an organism
- **Allele** - variant form of a given gene
- **Sequencing** - process of determining the order of nucleotides in a DNA sample
- **Shotgun sequencing** - a specific method for performing sequencing. Takes many small "reads" and assembles them into a continuous sequence
- **Genetic marker** - presence or absence of a gene at a specific locus known to increase the chance of some trait in the organism

This code performs two tasks:

- **Indexing** - creating an index of a genome, i.e. representing the genome in a format that allows doing faster look-ups.
- **Mapping** - finding the loci of genes and the distances between them on a chromosome (and a genome as a whole)

# Inputs and Outputs

When indexing we need to input a genome or a chromosome. This results in an index file as an output. This file is generated once and reused for all subsequent mappings.

The inputs to the mapping are the index file and a "read" of sequenced DNA. This outputs the loci of the given DNA.

## File Formats

- .gem - Index files from the GEM mapper
- FASTA/ FASTQ - text format for representing nucleotide or protein sequences. Can also be used to represent whole genomes. FASTQ contains quality data too
- SAM (sequence alignment map) - text format for sequences aligned against a reference sequence
- BED - 





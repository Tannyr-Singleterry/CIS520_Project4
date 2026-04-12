#!/bin/bash
#SBATCH --job-name=hw4-pthread
#SBATCH --partition=batch
#SBATCH --constraint=moles
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --mem-per-cpu=1G
#SBATCH --time=02:00:00
#SBATCH --output=hw4_pthread_%j.out

module purge
module load GCC

make

INPUT=~eyv/cis520/wiki_dump.txt
THREADS=$SLURM_CPUS_PER_TASK

./hw4-pthread $INPUT $THREADS
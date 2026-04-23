#!/bin/bash
#SBATCH --job-name=hw4_mpi_perf
#SBATCH --constraint=moles
#SBATCH --nodes=1
#SBATCH --ntasks=20
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=512M
#SBATCH --time=08:00:00
#SBATCH --output=perf_mpi_%j.out
#SBATCH --error=perf_mpi_%j.err

INPUT=~eyv/cis520/wiki_dump.txt
BIN=./hw4-mpi
CSV=perf_data_mpi_${SLURM_JOB_ID}.csv

# Compile
echo "Compiling..."
make
echo "Binary: $(ls -lh $BIN)"
echo "Node: $(hostname)"
echo "Tasks available: $SLURM_NTASKS"
uname -a

# CSV Header
echo "procs,nodes,mem_budget_mb,wall_time_s,user_time_s,sys_time_s,mem_used_kb,exit_code,trial" > $CSV


# Run experiments
for PROCS in 1 2 4 8 16 20; do
  for MEM in 64 128 512 1024 1536 3072; do
    for TRIAL in 1 2 3; do
      echo "--- procs=$PROCS mem=${MEM}MB trial=$TRIAL ---"

      /usr/bin/time -f "%e,%U,%S,%M" \
        mpirun -np $PROCS $BIN $INPUT > /dev/null \
        2>/tmp/hw4_time_$$.txt
      EXIT_CODE=$?

      TIME_LINE=$(cat /tmp/hw4_time_$$.txt)
      WALL=$(echo "$TIME_LINE" | cut -d',' -f1)
      USR=$(echo  "$TIME_LINE" | cut -d',' -f2)
      SYS=$(echo  "$TIME_LINE" | cut -d',' -f3)
      MEMKB=$(echo "$TIME_LINE" | cut -d',' -f4)

      echo "$PROCS,$SLURM_NNODES,$MEM,$WALL,$USR,$SYS,$MEMKB,$EXIT_CODE,$TRIAL" >> $CSV
      echo "  -> wall=${WALL}s mem=${MEMKB}KB exit=$EXIT_CODE"
    done
  done
done

rm -f /tmp/hw4_time_$$.txt
echo ""
echo "Job Finished. Results in $CSV"
echo "First few rows:"
head -10 $CSV
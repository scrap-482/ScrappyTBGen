#!/bin/bash

## Slurm batch file for the MPI experimentation

## Currently each mpi process independently computes its checkmates serially.
## This could be improved with greater thread level parallelism. However,
## for experimentation it is sufficient to launch multiple mpi processes on one node
## for checkmate identification instead of reserving many nodes to only run a single-threaded process.

#SBATCH --job-name=mpi_retrograde_analysis
#SBATCH --output=logs/output.%j 
#SBATCH --error=logs/error.%j  
#SBATCH --time=0-00:05:00     
#SBATCH --nodes=2            
#SBATCH --ntasks-per-node=48
#SBATCH --mem=16G  
#SBATCH --cpus-per-task=1  
#SBATCH --exclusive 
### Display some diagnostic information

mpirun -np 64 ./compiled/scrappytbgen >> results/retro_analysis_mpi_3man
echo '=====================JOB DIAGNOTICS========================'
date
echo -n 'This machine is ';hostname
echo -n 'My jobid is '; echo $SLURM_JOBID
echo 'My path is:' 
echo $PATH
sinfo -s
echo 'My job info:'
squeue -j $SLURM_JOBID
echo 'Machine info'
echo ' '
echo '========================ALL DONE==========================='

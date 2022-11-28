#!/bin/bash

## SLURM bash file for the single node checkmate identification experimentation.

#SBATCH --job-name=omp_checkmate_identification 
#SBATCH --output=logs/output.%j        
#SBATCH --error=logs/error.%j          
#SBATCH --time=0-00:05:00       
#SBATCH --nodes=1               
#SBATCH --ntasks-per-node=1     
#SBATCH --mem=16G  
#SBATCH --cpus-per-task=48  
#SBATCH --exclusive        
### Display some diagnostic information

## configure this parameter based on the number of threads to schedule
export OMP_NUM_THREADS=2
./compiled/scrappytbgen >> results/checkmate_omp
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

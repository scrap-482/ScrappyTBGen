#!/bin/bash

## SLURM bash file for the single node checkmate identification experimentation.

#SBATCH --job-name=omp_checkmate_identification 
#SBATCH --output=logs/output.%j        
#SBATCH --error=logs/error.%j          
#SBATCH --time=0-00:40:00       
#SBATCH --nodes=1               
#SBATCH --ntasks-per-node=1     
#SBATCH --mem=32G
#SBATCH --cpus-per-task=48  
#SBATCH --exclusive        
### Display some diagnostic information

## configure this parameter based on the number of threads to schedule
export OMP_NUM_THREADS=48
./compiled/scrappytbgen ##>> results/retro_analysis_omp_4man
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

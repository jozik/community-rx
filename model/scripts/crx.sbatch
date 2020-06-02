#!/bin/bash

#SBATCH --time=02:30:00
#SBATCH --partition=broadwl
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --exclusive

module load mpich/3.2
module load gcc/6.2
module load valgrind

export OMP_NUM_THREADS=28
CRX_MODEL=/project/jozik/midway2/repos/community-rx/model/Release/crx_model-0.3

#valgrind --tool=massif $CRX_MODEL ./config.props
#valgrind --tool=callgrind --instr-atstart=no $CRX_MODEL ./config.props
$CRX_MODEL ./config.props

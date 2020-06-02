#! /usr/bin/env bash
set -eu

if [ "$#" -ne 2 ]; then
  script_name=$(basename $0)
  echo "Usage: ${script_name} EXPERIMENT_ID CONFIG_FILE"
  exit 1
fi

EXP_ID=$1
CONFIG_FILE=$2

THIS=$( cd $( dirname $0 ) ; /bin/pwd )
ROOT="$THIS/.."
EXP_DIR="$ROOT/experiments/$EXP_ID"

mkdir -p $EXP_DIR
cp crx.sbatch $EXP_DIR
cp $CONFIG_FILE $EXP_DIR/config.props
cp "$(readlink -f $0)" $EXP_DIR

cd $EXP_DIR
#$PBT_FILE $PARAMS_FILE $EXP_DIR $MODEL_NAME $EXP_ID
sbatch --job-name=$EXP_ID crx.sbatch

cd $THIS

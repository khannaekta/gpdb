#!/bin/bash -l

set -eox pipefail

pushd gpdb_src/gpAux/gpdemo
export MASTER_DATA_DIRECTORY=`pwd`"/datadirs/qddir"

tar -xzf bin_gpdb/gpdb_log.tar.gz -C $MASTER_DATA_DIRECTORY/pg_log
